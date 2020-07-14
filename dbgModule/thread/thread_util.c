/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** thread_util.c
 *
 *	thread safe utility
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
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
#include	<sys/mman.h>
#include 	<limits.h>

#include	"ext_common.h"
#include	"threadlib.h"
#include	"thread_info.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define APP_MAPS_SZ			(128*3072)				/*3 Kbytes for 1 Mega Pool*/

#define REG_OPT_READ		0x01
#define	REG_OPT_WRITE		0x02
#define	REG_OPT_EXEC		0x04
#define REG_OPT_SHARED		0x08

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define	REG_OPT_TEXT		(REG_OPT_READ|              REG_OPT_EXEC)
#define	REG_OPT_ALL			(REG_OPT_READ|REG_OPT_WRITE|REG_OPT_EXEC)


/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef enum
{
	METHOD_AUTO   = 0,
	METHOD_PROCFS = METHOD_AUTO,
	METHOD_LGDRV,
	METHOD_MAX
} MAP_METHOD_T;

typedef struct
{
	int			incCnt;
	int			mapNum;
	region_t	pMapData[1];
} map_info_t;

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static pthread_mutex_t	_regionMtx      	= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static bool				_map_initialized	= false;
static MAP_METHOD_T		_mapMethod			= METHOD_AUTO;
static map_info_t*		_pMapInfo			= NULL;
static region_t*		_pRegionLst 		= NULL;
static int				_maps_fd			= -1;


/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static void _lockRegion(void)
{
	(void) pthread_mutex_lock(&_regionMtx);
	return;
}

static void _unlockRegion(void)
{
	(void) pthread_mutex_unlock(&_regionMtx);
	return;
}


static void _region_map_init(void)
{
	char				buff[4096], mode[256];
	uint64_t			beg, end, opt;
	static FILE			*fp = NULL;
	static int _TLS_	nest = 0;


	if(!TLS_VALID(nest))
		return;

	if(_mapMethod != METHOD_PROCFS)
		return;

	_lockRegion();
	nest++;

	if(fp == NULL)
	{
		fp = fopen("/proc/self/maps", "r");
	}
	rewind(fp);
	_pMapInfo->mapNum = 0;

	if(fp == NULL)
		goto _END_FUNC;

	while(fgets(buff, sizeof(buff), fp))
	{
		if ((_pMapInfo->mapNum * sizeof(region_t) + sizeof(map_info_t)) > APP_MAPS_SZ)
			break;

		sscanf(buff, "%016lx-%016lx %s", &beg, &end, mode);
		opt = 0;
		if (mode[0] == 'r') opt |= REG_OPT_READ;
		if (mode[1] == 'w') opt |= REG_OPT_WRITE;
		if (mode[2] == 'x') opt |= REG_OPT_EXEC;
		if (mode[3] == 's') opt |= REG_OPT_SHARED;
		if (opt != 0)
		{
			_pRegionLst[_pMapInfo->mapNum].reg_beg = beg;
			_pRegionLst[_pMapInfo->mapNum].reg_end = end;
			_pRegionLst[_pMapInfo->mapNum].reg_opt = opt;
			_pMapInfo->mapNum++;
		}
	}

_END_FUNC:
	if(nest)
		nest--;
	_unlockRegion();
}

static region_t _getRegionEntry_ext(uint64_t addr, int opt)
{
	int			iIncCnt, iMapNum;
	region_t	retRegion = {
			.reg_beg = 0,
			.reg_end = 0,
			.reg_opt = 0};


	if(addr == 0)
		return retRegion;

	if(_mapMethod == METHOD_LGDRV)
	{
		uint64_t	offs = lseek(_maps_fd, addr, SEEK_SET);
		int			sz   = 0;

		if(offs != addr)
			return retRegion;

		retRegion.reg_beg = offs;
		sz = read(_maps_fd, &retRegion, sizeof(region_t));
		if(sz != sizeof(region_t))
		{
			retRegion.reg_beg = 0;
			retRegion.reg_end = 0;
			retRegion.reg_opt = 0;
		}

		return retRegion;
	}

	if(_pMapInfo == NULL || _pRegionLst == NULL)
		return retRegion;

	_lockRegion();

	if(_mapMethod == METHOD_PROCFS)
		_region_map_init();

	if      (addr >= _pRegionLst[0].reg_beg && addr < _pRegionLst[0].reg_end)
	{
		_unlockRegion();
		return _pRegionLst[0];
	}
	else if (addr < _pRegionLst[0].reg_beg)
	{
		_unlockRegion();
		return retRegion;
	}

	do
	{
		int	l, r, c;

		iIncCnt = _pMapInfo->incCnt;
		iMapNum = _pMapInfo->mapNum;
		c = 0;
		l = 1;
		r = iMapNum;

		while (l <= r)
		{
			c = (l + r) / 2;
			if      (addr <  _pRegionLst[c].reg_beg)  { r = c - 1; }
			else if (addr >= _pRegionLst[c].reg_end)  { l = c + 1; }
			else if (iIncCnt != _pMapInfo->incCnt   ) { break;     }
			else
			{
				retRegion = _pRegionLst[c];
				break;
			}
		}
		if ((l > r) || (iIncCnt == _pMapInfo->incCnt))
		{
			if ((l > r) && (opt != 0))
			{
				retRegion.reg_beg = _pRegionLst[r].reg_end;
				retRegion.reg_end = _pRegionLst[l].reg_beg;
				retRegion.reg_opt = 0;
			}

			_unlockRegion();
			return retRegion;
		}

	} while(1);

	_unlockRegion();
	return retRegion;
}

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void thread_map_init(void)
{
	if(_map_initialized == true)
		return;

	/*TODO: After work of lg driver
	 *      should use kernel lg driver method
	 */

	if((_maps_fd = get_drv_fd()) < 0)
		_mapMethod = METHOD_PROCFS;
	else
		_mapMethod = METHOD_LGDRV;

	if (_mapMethod == METHOD_PROCFS)
	{
		_pMapInfo   = (map_info_t *)mmap((void*)0, APP_MAPS_SZ, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
		_pRegionLst = (region_t   *)&_pMapInfo->pMapData[0];
		_pMapInfo->incCnt = 0;
		_pMapInfo->mapNum = 0;
	}

	_map_initialized = true;
}

region_t _getRegionEntry(uint64_t addr)
{
	return _getRegionEntry_ext(addr, 0);
}
STRONG_ALIAS(_getRegionEntry, getRegionEntry);

bool isValidAddr(uint64_t addr)
{
	region_t mReg = _getRegionEntry(addr);
	return (((mReg.reg_opt & REG_OPT_READ) == REG_OPT_READ) ?  true : false);
}

bool isValidPc(uint64_t pc)
{
	region_t	mReg;

	if(_mapMethod == METHOD_LGDRV)
	{
		static region_t mReg_Cache;
		if(mReg_Cache.reg_beg <= pc && pc < mReg_Cache.reg_end)
		{
			mReg = mReg_Cache;
		}
		else
		{
			mReg = _getRegionEntry(pc);
			mReg_Cache = mReg;
		}

		if ((mReg.reg_opt & REG_OPT_ALL) == REG_OPT_TEXT)
			return true;
		return false;
	}
	if (pc < _pRegionLst[0].reg_end)
	{
		extern int _init[], _fini[];

		if      (pc >  (uint64_t)_fini       ) return false;
		else if (pc >= (uint64_t)_init       ) return true;
		else if (pc >= _pRegionLst[0].reg_beg) return false;
	}

	mReg = _getRegionEntry(pc);
	if ((mReg.reg_opt & REG_OPT_ALL) == REG_OPT_TEXT)
		return true;
	return false;
}
