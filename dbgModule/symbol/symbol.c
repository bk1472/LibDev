/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** symbol.c
 *
 *	symbol init & search logic
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 29
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/mman.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#include	"symbol.h"

#ifdef CPP_DEMANGLE
#include	"demangle.h"
#endif
/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define		MAX_APPNM			(256)
#define		MOD_64				4
#define		MOD_32				2
#define		MOD_16				0

#define		DWARF_MARK			1

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define		SYM_MAGIC			(0xB12791EEEE19721B)

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	uint64_t	magic;
	uint64_t	nBss;
	uint64_t	totSize;
	uint64_t	symNum;
	uint64_t	symStrTblSz;
	uint64_t	location_symtab;
} symStorage_t;


typedef struct {
	uint64_t	addr;
	uint64_t	end;
	uint64_t	ptr;
} symEntry_t;

/*-----------------------------------------------------------------------------
 *	External Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
extern unsigned char*	pDwarfData;
extern uint64_t			nDwarfLst;
extern uint64_t*		dwarfLst;

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static const char*		_pNoSymMsg   = "No symbol table";

static bool				bSymLoad    = false;
static char				appName[MAX_APPNM] ={0};
static char				filName[MAX_APPNM] ={0};

static char*			_pLibName    = NULL;

uint64_t				nTxtSyms     = 0;
uint64_t*				pSymTabBase  = NULL;
uint64_t*				pSymHashBase = NULL;
char*					pSymStrBase  = NULL;


/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static uint64_t _load_sym_data(char *appName)
{
	static int		fd          = -1;
	static uint64_t*ptmp        = MAP_FAILED;
	static uint64_t	nBytes      = 0;

	char*			cp;
	struct stat		sb;
	uint64_t		len;

	symStorage_t*	pSymStorage = NULL;

	if( (fd > 0) && (ptmp != MAP_FAILED) && (nBytes != 0))
		return nBytes;

	if((cp = strrchr(appName, '.')) != NULL) len = cp - appName;
	else                                     len = strlen(appName);

	snprintf(filName, MAX_APPNM, "%s.sym", appName);
	#if 0
	rprint1n("^B^Checking & Importing debug symbols from file %s", filName);
	#endif

	if( (fd = open(filName, O_RDONLY, 0777)) < 0)
		return 0;

	if (fstat(fd, &sb) == 0)	len = sb.st_size;
	else                        len = 0;

	if(0 == len)
	{
		goto _RET_ERR_CLOSE;
	}

	if((ptmp = (void*)mmap((void*)0, len, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
	{
		goto _RET_ERR_CLOSE;
	}

	nBytes      = len;
	pSymStorage = (symStorage_t*)ptmp;
	if(pSymStorage->magic == SYM_MAGIC)
	{
		uint64_t	bitSz;
		uint64_t	dwarfMark = 0;

		uint64_t	val1 = pSymStorage->totSize;
		uint64_t	val2 = pSymStorage->symNum;
		uint64_t	val3 = pSymStorage->symStrTblSz;
		uint64_t	val4 = 3*sizeof(uint64_t)*val2 + val3;
		uint64_t	val9 = val1 + 5 * sizeof(uint64_t);

		if ((nBytes != val9) || (val1 != val4))
		{
			nBytes = 0; goto _RET_ERR_MUNMAP;
		}

		nTxtSyms    = pSymStorage->symNum;
		pSymTabBase = &pSymStorage->location_symtab;
		pSymStrBase = (char *)(pSymTabBase + 3 * pSymStorage->symNum);
		bitSz       = *(uint64_t*)pSymStrBase;

		/* 64 bit Mode */
		if (bitSz != MOD_64)
		{
			nBytes = 0; goto _RET_ERR_MUNMAP;
		}

		pSymHashBase = (uint64_t*)(pSymStrBase + sizeof(uint64_t));
		pSymStrBase  = (char*)(pSymHashBase + ((nTxtSyms + 1) & ~1));
		dwarfMark = *(uint64_t*)pSymStrBase;

		if(dwarfMark != DWARF_MARK)
		{
			nBytes = 0; goto _RET_ERR_MUNMAP;
		}
		nDwarfLst   = ((uint64_t*     )pSymStrBase)[1];
		val2        = ((uint64_t*     )pSymStrBase)[2];
		dwarfLst    = (uint64_t*      )(pSymStrBase + 3*sizeof(uint64_t));
		nDwarfLst   = ((uint64_t*     )pSymStrBase)[1];
		pDwarfData	= (unsigned char *)(dwarfLst + 2 * nDwarfLst);
		pSymStrBase = (char*)(pDwarfData + val2);


		rprint1n("^G^64bit symbol Hash Mode");
		rprint1n("^B^nTxtSyms     = %lu", nTxtSyms);
		rprint1n("^B^pSymTabBase  = [%p..%p)", pSymTabBase,  pSymTabBase + 3 * nTxtSyms);
		rprint1n("^B^pSymHashBase = [%p..%p)", pSymHashBase, pSymHashBase + nTxtSyms);
		rprint1n("^B^pSymStrBase  = [%p..%p)", pSymStrBase,  ptmp + val9);

		rprint1n("^B^nDwarfLst    = %lu", nDwarfLst);
		rprint1n("^B^pDwarfLst    = [%p..%p)", dwarfLst, dwarfLst + 2 * nDwarfLst);
		rprint1n("^B^pDwarfData   = [%p..%p)", pDwarfData, pDwarfData + val2);

		goto _RET_OK;
	}

_RET_ERR_MUNMAP:
	munmap(ptmp, len);
	ptmp = MAP_FAILED;

_RET_ERR_CLOSE:
	close(fd);
	fd = -1;

	nTxtSyms     = 0;
	nDwarfLst    = 0;
	pSymTabBase  = NULL;
	pSymHashBase = NULL;
	pSymStrBase  = NULL;
	pDwarfData   = NULL;
	dwarfLst     = NULL;

_RET_OK:

	return nBytes;
}

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void init_symbol(void)
{
	int rc;

	if(bSymLoad == true)
		return;

	if(appName[0] == '\0')
	{
		rc = readlink("/proc/self/exe", &appName[0], MAX_APPNM);
		if(rc == -1)
		{
			appName[0] = '\0';
			return;
		}
	}
	if((_load_sym_data(&appName[0])) == 0)
		return;

	bSymLoad = true;
}

bool is_symbol_load(void)
{
	return bSymLoad;
}

char* getLibName(void)
{
	return _pLibName;
}

uint64_t findDmgSymByAddr(uint64_t addr, char **pSymName, char* dmgBuf, int size)
{
	int			x,l = 0, r = nTxtSyms-1, matched = 0;
	symEntry_t	*pSyms = (symEntry_t*)pSymTabBase;

	if(bSymLoad == false)
		init_symbol();

	if (pSymName == NULL)
		return 0;

	if (pSyms == NULL)
	{
		*pSymName = (char*)_pNoSymMsg;
		return 0;
	}

	_pLibName = NULL;
	do
	{
		x = (l + r) / 2;
		if      (addr < pSyms[x].addr) { matched = 0; r = x - 1; }
		else if (addr < pSyms[x].end ) { matched = 1; l = x + 1; }
		else                           { matched = 0; l = x + 1; }

	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		*pSymName = &pSymStrBase[pSyms[x].ptr];
		#ifdef CPP_DEMANGLE
		if( (dmgBuf != NULL) && (size != 0))
		{
			if (NULL != (demangle_symbol((char*)*pSymName, &dmgBuf[0], size)))
				*pSymName = &dmgBuf[0];
		}
		#endif
		return(pSyms[x].addr);
	}
	else
	{
		return findDynSymByAddr(addr, pSymName, &_pLibName);
	}

}

uint64_t findSymByAddr(uint64_t addr, char **pSymName)
{
	return findDmgSymByAddr(addr, pSymName, NULL, 0);
}

char *findSymbolName(uint64_t addr)
{
	char		*pSymName;
	static char nameBuf[20];

	if(bSymLoad == false)
		init_symbol();

	if (findSymByAddr(addr, &pSymName) != 0)
		return pSymName;

	snprintf(nameBuf, 20, "%#lx", addr);
	return(nameBuf);

}

uint64_t findSymByName(const char *symName)
{
	int				x, l = 0, r = nTxtSyms-1, matched = 0, rc;
	uint64_t		addr = 0;
	symEntry_t		*pSyms = (symEntry_t *)pSymTabBase;

	if(bSymLoad == false)
		init_symbol();

	_pLibName = NULL;
	do
	{
		x = (l + r) / 2;
		rc = strcmp(symName, &pSymStrBase[pSyms[pSymHashBase[x]].ptr]);
		if      (rc < 0) { matched = 0; r = x - 1; }
		else if (rc > 0) { matched = 0; l = x + 1; }
		else             { matched = 1;            }

	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		addr = pSyms[pSymHashBase[x]].addr;
	}
	else
	{
		addr = findDynSymByName(symName, &_pLibName);
	}

	return(addr);
}
