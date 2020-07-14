/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @dyn_sym.c
 *
 *	get dysnamic symbol information from SO libraries
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 12
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/
#define DYNDBG_LVL		0

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"symbol.h"
#include	"thread.h"
#include	"xlibc.h"

#if (USE_DYM_SYMBOL > 0)
#include	<elf.h>
#include	<errno.h>
#include	<link.h>
#include	<unistd.h>
#include	<sys/mman.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct _sym_info_
{
	uint64_t	addr[2];
	char*		ptr;
} sym_info_t;

typedef struct _lib_info_
{
	struct _lib_info_*	next;
	struct _lib_info_*	prev;

	int					nSym;
	char*				pLibName;
	uint64_t			mapsAddr[2];
	uint64_t*			pHashStr;
	uint64_t*			pHashAddr;
	sym_info_t*			pSymTbl;
} LIB_INFO_t;

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static LIB_INFO_t*		_pLibInfo[2] = {(LIB_INFO_t*)&_pLibInfo[0], (LIB_INFO_t*)&_pLibInfo[0]};
static LIB_INFO_t*		_pLibSort    = NULL;
static bool				_isLoad      = false;

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static int _addLibInfo(LIB_INFO_t* p_new, LIB_INFO_t* p_header)
{
	LIB_INFO_t*	p_base;

	p_base = LS_BASE(LIB_INFO_t, p_header);

	LS_INS_BEFORE(p_base, p_new);

	return 0;
}

static int _compareStr(const void *a, const void *b)
{
	sym_info_t	*p_si;
	int			res;

	if(_pLibSort == NULL)
	{
		rprint1n("^R^Data Pointer is not implemented!");
		return 0;
	}

	p_si = _pLibSort->pSymTbl;

	res = strcmp(p_si[*(uint64_t*)a].ptr, p_si[*(uint64_t*)b].ptr);

	return ( ( (res > 0) ? 1 : ((res ==  0) ? 0 : -1) ) );
}

static int _compareAddr(const void *a, const void *b)
{
	sym_info_t*	p_si;
	int			res = 0;

	if(_pLibSort == NULL)
	{
		rprint1n("^R^Data Pointer is not implemented!");
		return 0;
	}

	p_si = _pLibSort->pSymTbl;

	if     (p_si[*(uint64_t*)a].addr[0] > p_si[*(uint64_t*)b].addr[0])
		res = 1;
	else if(p_si[*(uint64_t*)a].addr[0] < p_si[*(uint64_t*)b].addr[0])
		res = -1;
	else
		res = 0;

	return res;
}

static LIB_INFO_t* _find_LibInfo(uint64_t maps_info, LIB_INFO_t* p_header)
{
	LIB_INFO_t*	p_base;
	LIB_INFO_t*	p_info;

	p_base = LS_BASE(LIB_INFO_t, p_header);

	if(LS_ISEMPTY(p_base))
		return NULL;

	p_info = p_base->next;

	while(p_info != p_base)
	{
		if( p_info->mapsAddr[0] <= maps_info && p_info->mapsAddr[1] >= maps_info )
			return p_info;
		p_info = p_info->next;
	}

	return NULL;
}

static int _phdr_callback(struct dl_phdr_info* p_info, size_t size, void* p_data)
{
	LIB_INFO_t*		p_libInfo = NULL;
	int				realSymCnt = 0;
	int				n;

	if(p_data == NULL)
		return -1;

	for(n = 0; n < p_info->dlpi_phnum; n++)
	{
		Elf64_Dyn*	p_dyn    = NULL;
		Elf64_Sym*	p_sym    = NULL;
		char*		p_strtab = NULL;
		size_t		nSym;
		int			l;
		region_t	mReg;

		if(p_info->dlpi_phdr[n].p_type != PT_DYNAMIC)
			continue;

		if(p_info->dlpi_addr == 0)
		{
			return 0;
		}

		if((p_libInfo = _find_LibInfo((uint64_t)p_info->dlpi_addr, (LIB_INFO_t*)p_data)) != NULL)
		{
			#if (DYNDBG_LVL > 0)
			rprint1n("^y^%-40.40s is already registed!", p_info->dlpi_name);
			#endif
			return 0;
		}

		p_libInfo = (LIB_INFO_t*) mmap((void *) NULL, sizeof(LIB_INFO_t), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		p_dyn     = (Elf64_Dyn *) (p_info->dlpi_addr + p_info->dlpi_phdr[n].p_vaddr);

		for(l = 0; p_dyn[l].d_tag != DT_NULL; ++l)
		{
			switch(p_dyn[l].d_tag)
			{
				case DT_STRTAB:
					if(p_dyn[l].d_un.d_ptr > p_info->dlpi_addr)
						p_strtab =      (char*)(                    p_dyn[l].d_un.d_ptr);
					else
						p_strtab =      (char*)(p_info->dlpi_addr + p_dyn[l].d_un.d_ptr);
					break;
				case DT_SYMTAB:
					if(p_dyn[l].d_un.d_ptr > p_info->dlpi_addr)
						p_sym    = (Elf64_Sym*)(                    p_dyn[l].d_un.d_ptr);
					else
						p_sym    = (Elf64_Sym*)(p_info->dlpi_addr + p_dyn[l].d_un.d_ptr);
					break;
			}
		}

		if(p_strtab == NULL || p_sym == NULL)
		{
			rprint1n("^R^Strtab is NULL");
			munmap(p_libInfo, sizeof(LIB_INFO_t));
			return -1;
		}

		nSym = (Elf64_Sym*)p_strtab - p_sym;

		#if (DYNDBG_LVL > 0)
		rprint1n("^y^%-40.40s(0x%lx):0x%lx, phdr[0x%lx], phnum.%02d, nSym: %07d",
			p_info->dlpi_name, (uint64_t)p_info->dlpi_name, p_info->dlpi_addr, (uint64_t)p_info->dlpi_phdr, p_info->dlpi_phnum, nSym);
		#endif

		mReg = getRegionEntry((uint64_t)p_info->dlpi_phdr+sizeof(uint64_t));
		if(mReg.reg_beg != 0)
		{
			p_libInfo->mapsAddr[0] = mReg.reg_beg;
			p_libInfo->mapsAddr[1] = mReg.reg_end;
		}
		else
		{
			rprint1n("^R^%-40.40s can't find in Region List", p_info->dlpi_name);
			munmap(p_libInfo, sizeof(LIB_INFO_t));
			return -2;
		}

		p_libInfo->pLibName  = (char*)p_info->dlpi_name;
		p_libInfo->nSym      = nSym;
		p_libInfo->pHashStr  = (uint64_t   *) mmap((void *) NULL, nSym*sizeof(uint64_t),   PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		p_libInfo->pHashAddr = (uint64_t   *) mmap((void *) NULL, nSym*sizeof(uint64_t),   PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		p_libInfo->pSymTbl   = (sym_info_t *) mmap((void *) NULL, nSym*sizeof(sym_info_t), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

		for(l = 0, realSymCnt = 0; l < nSym; ++l)
		{
			char		*pSymbolName;
			uint64_t	*pAddr;
			size_t		_Sz;

			#if 0
			if(ELF32_ST_TYPE(p_sym[l].st_info) != STT_FUNC ||
			   ELF32_ST_BIND(p_sym[l].st_info) != STB_GLOBAL)
				continue;
			#endif

			pSymbolName = p_strtab + p_sym[l].st_name;
			pAddr       = (uint64_t*)(p_info->dlpi_addr + p_sym[l].st_value);
			_Sz         = p_sym[l].st_size;

			p_libInfo->pHashStr [realSymCnt]           = realSymCnt;
			p_libInfo->pHashAddr[realSymCnt]           = realSymCnt;
			p_libInfo->pSymTbl  [realSymCnt].addr[0] = (uint64_t)pAddr;
			p_libInfo->pSymTbl  [realSymCnt].addr[1] = (uint64_t)pAddr + (uint64_t)_Sz;
			p_libInfo->pSymTbl  [realSymCnt].ptr       = (char*)pSymbolName;

			realSymCnt++;
		}
	}
	p_libInfo->nSym  = realSymCnt;

	_pLibSort = p_libInfo;
	qsort(&p_libInfo->pHashStr[0],  p_libInfo->nSym, sizeof(uint64_t), _compareStr);
	qsort(&p_libInfo->pHashAddr[0], p_libInfo->nSym, sizeof(uint64_t), _compareAddr);
	_pLibSort = NULL;

	#if (DYNDBG_LVL > 2)
	{
	  sym_info_t*	p_si   = p_libInfo->pSymTbl;
	  uint64_t*		p_hash = p_libInfo->pHashStr;
	  int			_cnt   = realSymCnt;

	  for(n = 0; n < _cnt; n++)
	  {
		uint64_t	_addr[2];
		char*		pStr;
	  	_addr[0] = p_si[p_hash[n]].addr[0];
		_addr[1] = p_si[p_hash[n]].addr[1];
		pStr     = p_si[p_hash[n]].ptr;

	  	#if   (DYNDBG_LVL == 3)
	  	rprint1n("^c^[%lx-%lx] %-50.50s", _addr[0], _addr[1], pStr);
	  	#elif (DYNDBG_LVL == 4)
	  	rprint1n("^c^[%lx-%lx] %lx",      _addr[0], _addr[1], (uint64_t)pStr);
	  	#endif
	  }
	}
	#endif

	_addLibInfo(p_libInfo, (LIB_INFO_t*)p_data);

	_isLoad = true;
	return 0;
}

static int _load_dynamic_symbol(LIB_INFO_t* p_info)
{
	return dl_iterate_phdr(_phdr_callback, (void*)p_info);
}

static int _load_dyn_symbol(void)
{
	if(false != _isLoad)
		return -1;

	return _load_dynamic_symbol((LIB_INFO_t*)_pLibInfo);
}
/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
uint64_t findDynSymByAddr(uint64_t addr, char **pSymName, char **pLibName)
{
	LIB_INFO_t*	p_libInfo;
	sym_info_t*	p_symInfo;
	uint64_t*	p_hash;
	int			nSym;
	int			x, l = 0, r, matched = 0;

	if (_load_dyn_symbol() < 0)
		return 0;

	if (pSymName == NULL)
		return 0;

	p_libInfo = _find_LibInfo(addr, (LIB_INFO_t*)_pLibInfo);

	if(p_libInfo == NULL)
	{
		*pSymName = (char *)"No symbol table";
		return 0;
	}
	p_hash = p_libInfo->pHashAddr;
	p_symInfo   = p_libInfo->pSymTbl;
	nSym  = p_libInfo->nSym;

	r = nSym -1;

	do
	{
		x = (l + r) / 2;
		if      (addr < p_symInfo[p_hash[x]].addr[0]) { matched = 0; r = x - 1; }
		else if (addr < p_symInfo[p_hash[x]].addr[1]) { matched = 1; l = x + 1; }
		else                                          { matched = 0; l = x + 1; }

	} while((l <= r) && (matched == 0));

	if(matched)
	{
		if(pLibName)
		{
			#if 0
			dbgprint("^Y^%p", p_libInfo->pLibName);
			#endif
			*pLibName = p_libInfo->pLibName;
		}
		*pSymName = p_symInfo[p_hash[x]].ptr;
		return (p_symInfo[p_hash[x]].addr[0]);
	}

	*pSymName = (char *)"Not Found";
	return 0;
}

uint64_t findDynSymByName(const char *symName, char **pLibName)
{
	LIB_INFO_t*	p_base, *p_libInfo;
	uint64_t	addr = 0;

	if (_load_dyn_symbol() < 0)
		return 0;

	p_base    = LS_BASE(LIB_INFO_t, _pLibInfo);
	p_libInfo = p_base->next;

	while(p_libInfo != p_base)
	{
		sym_info_t*	p_symInfo = p_libInfo->pSymTbl;;
		uint64_t*	p_hash    = p_libInfo->pHashStr;
		int			nSym      = p_libInfo->nSym;
		int			x, l = 0, r = nSym -1, matched = 0, rc;

		do
		{
			x = (l + r) / 2;
			rc = strcmp(symName, (char*)p_symInfo[p_hash[x]].ptr);
			if      (rc < 0) { matched = 0; r = x - 1; }
			else if (rc > 0) { matched = 0; l = x + 1; }
			else             { matched = 1;            }

		}while ((l <= r) && (matched == 0));

		if (matched)
		{
			addr = p_symInfo[p_hash[x]].addr[0];
			break;
		}

		p_libInfo = p_libInfo->next;
	}

	if(pLibName)
	{
		#if 0
		dbgprint("^Y^%p", p_libInfo->pLibName);
		#endif
		*pLibName = p_libInfo->pLibName;
	}
	return addr;
}

char* findDynSymbolName (uint64_t addr)
{
	char		*pSymName;
	static char	nameBuf[18];

	if (findDynSymByAddr(addr, &pSymName, NULL) != 0)
		return pSymName;
	snprintf(nameBuf, 18, "%#lx", addr);

	return (&nameBuf[0]);
}

#else
uint64_t findDynSymByAddr(uint64_t addr, char **pSymName, char **pLibName)
{
	if (NULL == pSymName)
		return -1;
	*pSymName = "Not Found";
	return 0;
}

uint64_t findDynSymByName(const char *symName, char **pLibName)
{
	return 0;
}

char* findDynSymbolName (uint64_t addr)
{
	return "Not Found";
}
#endif
