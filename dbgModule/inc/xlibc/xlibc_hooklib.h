/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_hooklib.h
 *
 *	Hooking library file
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */
#ifndef __XLIBC_HOOKLIB_H__
#define __XLIBC_HOOKLIB_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/

#include	"ext_common.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct {
	char*	p_func;
	char*	p_lib;
} HOOK_LIST_t;


/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern printf_proc_t	orgPrintf;
extern fprintf_proc_t	orgFprintf;

/*-----------------------------------------------------------------------------
 * External Used Functions
------------------------------------------------------------------------------*/
/* xlibc_hook_func.c */
extern int		xlibc_getHookList			(HOOK_LIST_t**);
extern void		xlibc_backupHook			(void);

/* xlibc_hooklib.c */
extern void*	xlibc_reg_hook				(const char* , void*);
extern void*	xlibc_getOrigFunc			(const char*);
extern void*	xlibc_getHookFunc			(const char*);
extern int		xlibc_check_hook			(void);
extern void		xlibc_init_hook_system		(void); /*should be called in constructor function*/

#ifdef __cplusplus
}
#endif

#endif/*__XLIBC_HOOKLIB_H__*/
