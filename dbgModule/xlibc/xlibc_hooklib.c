/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_hooklib.c
 *
 *	Hook default system libraries
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include	<dlfcn.h>

#ifndef __HOOK_LIB__
#define __HOOK_LIB__
/**
 * BK1472 Comment :
 * do not compile hooking function indivisually
 */
#include "./xlibc_hook_func.c"
#endif
/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define HOOKTBL_MAX			30

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct _hook_tbl_
{
	char*	pFuncName;
	char*	pLIBName;

	void*	pFuncPtr;
	void*	pHookPtr;
	int		loaded;
} hook_tbl_t;

/*-----------------------------------------------------------------------------
 * Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static int			_hook_service_init = 0;
static hook_tbl_t	_hookTbl[HOOKTBL_MAX];

static int			_hookSortTbl[HOOKTBL_MAX];
static int			_maxCnt = HOOKTBL_MAX;

/*-----------------------------------------------------------------------------
 * Global Variables or Function Pointer
------------------------------------------------------------------------------*/
printf_proc_t		orgPrintf  = NULL;
fprintf_proc_t		orgFprintf = NULL;

/*-----------------------------------------------------------------------------
 * Static Function Definition
------------------------------------------------------------------------------*/
static int compare_hooktbl(const void *a, const void *b)
{
	int cmp = strcmp(_hookTbl[*(int*)a].pFuncName, _hookTbl[*(int*)b].pFuncName);

	return( ( (cmp > 0) ? 1 : ((cmp ==  0) ? 0 : -1) ) );
}

static int __init_hook_table(hook_tbl_t* pHook)
{
	char*	error  = NULL;
	void*	handle = NULL;
	int		rc     = -1;


	if(0 !=  pHook->loaded)
		return 0;

	if (pHook->pLIBName == NULL)
		handle = RTLD_NEXT;
	else
		handle = dlopen(pHook->pLIBName, RTLD_LAZY);

	if (handle == NULL)
		return -1;

	/*error clear*/
	dlerror();

	/* get original function pointer address */
	pHook->pFuncPtr = (void*) dlsym(handle, pHook->pFuncName);
	if ((error = dlerror()) != NULL)
	{
		pHook->pFuncPtr = (void*)null_function;
		goto _ERR_END;
	}

	pHook->loaded = 1;
	rc = 0;

_ERR_END:
	if (handle != RTLD_NEXT)
		dlclose(handle);

	return rc;
}

static void	_init_libc_hook(void)
{
	HOOK_LIST_t*	ppHookList = NULL;
	int		_nHookSz   = 0;
	int		n;

	if (0 != _hook_service_init)
		return;

	/**
	 * BK1472 Comment:
	 * run time initialize then hooking table
	 */
	_nHookSz = xlibc_getHookList(&ppHookList);
	if(_nHookSz > HOOKTBL_MAX)
		_nHookSz = HOOKTBL_MAX;

	if (ppHookList)
	{
		for(n = 0; n < _nHookSz; n++)
		{
			_hookTbl[n].pFuncName = ppHookList[n].p_func;
			_hookTbl[n].pLIBName  = ppHookList[n].p_lib;
			_hookTbl[n].pFuncPtr  = (void*)null_function;
			_hookTbl[n].pHookPtr  = NULL;
			_hookTbl[n].loaded    = 0;
		}
		_maxCnt = _nHookSz;
	}
	/**
	 * BK1472 comment : 2018/02/21
	 * 만약 _maxCnt 가 HOOKTBL_MAX 의 size 보다 크다면
	 * HOOKTBL_MAX 까지만 count 한다.
	 */

	for (n = 0; n < _maxCnt; n++)
	{
		_hookSortTbl[n] = n;
	}
	qsort(&_hookSortTbl[0], _maxCnt, sizeof(int), compare_hooktbl);

	for(n = 0; n < _maxCnt; n++)
	{
		if(__init_hook_table(&_hookTbl[n]) < 0)
			return;
	}

	_hook_service_init = 1;
}

static hook_tbl_t* _find_hook_item(const char *pfunc_name)
{
	int	x, l = 0, r = _maxCnt -1;
	int matched = 0;

	do
	{
		x = (l + r)/2;
		if      (strcmp(pfunc_name, _hookTbl[_hookSortTbl[x]].pFuncName) <  0) {matched = 0; r = x - 1;}
		else if (strcmp(pfunc_name, _hookTbl[_hookSortTbl[x]].pFuncName) == 0) {matched = 1; l = x + 1;}
		else                                                                   {matched = 0; l = x + 1;}
	} while((l <= r) && (matched == 0));

	if (matched)
	{
		return &(_hookTbl[_hookSortTbl[x]]);
	}

	return NULL;
}


static void* __get_dl_Function(const char* p_libnm, const char* p_name)
{
	char*	error  = NULL;
	void*	handle = NULL;
	void*	p_rt   = NULL;

	if (p_libnm == NULL)
		handle = RTLD_NEXT;
	else
		handle = dlopen(p_libnm, RTLD_LAZY);

	if(NULL == handle)
		return NULL;

	/*error clear*/
	dlerror();

	/* get original function pointer address */
	p_rt = (void*) dlsym(handle, p_name);
	if ((error = dlerror()) != NULL)
	{
		p_rt = NULL;
	}

	if(handle != RTLD_NEXT)
		dlclose(handle);

	return p_rt;
}



void xlibc_init_hook_system(void)
{
	_init_libc_hook();

	if(NULL == orgPrintf)
	{
		orgPrintf = __get_dl_Function(NULL, "printf");

		if(NULL == orgPrintf)
			orgPrintf = (printf_proc_t)rprint0n;
	}
	if(NULL == orgFprintf)
	{
		orgFprintf = __get_dl_Function(NULL, "fprintf");

		if(NULL == orgFprintf)
			orgFprintf = (fprintf_proc_t)rprint0n;
	}
	/**
	 * BK1472 Comment:
	 * backup original function
	 */
	xlibc_backupHook();
}

/*-----------------------------------------------------------------------------
 * API Function Definition
------------------------------------------------------------------------------*/
void* xlibc_reg_hook(const char* function_nm, void* pFunc)
{
	hook_tbl_t*	pHook = _find_hook_item(function_nm);

	if(pHook == NULL)
		return NULL;

	pHook->pHookPtr = pFunc;

	/* return original function pointer */
	return pHook->pFuncPtr;
}


void* xlibc_getOrigFunc(const char* p_name)
{
	hook_tbl_t*	pHook = _find_hook_item(p_name);

	if(pHook == NULL)
		return NULL;

	return pHook->pFuncPtr;
}

void* xlibc_getHookFunc(const char* p_name)
{
	hook_tbl_t*	pHook = _find_hook_item(p_name);

	if(pHook == NULL)
		return NULL;

	return pHook->pHookPtr;
}

int xlibc_check_hook(void)
{
	return _hook_service_init;
}
