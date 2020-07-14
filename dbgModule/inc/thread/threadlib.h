/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file threadlib.h
 *
 *	Thread library hook and extension
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
 *	@note
 *	@see
 */
#ifndef __THREADLIB_H__
#define __THREADLIB_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"thread_info.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
/*threadlib.c*/
extern void			init_thread_system			(void);
extern int			get_current					(void);

extern pid_t		get_tid						(int);
extern pthread_t	get_threadid				(int);
extern char*		get_threadName				(int);
extern int			get_tsk_id_by_name			(const char*);
extern void			thread_info_print			(int);
extern int			pthread_create_ext			(	const char* name,
													pthread_t *restrict thread,
											 		pthread_attr_t *restrict attr,
													void *(*start_routine)(void*),
													void *restrict arg);
extern void			regist_thread_info			(const char* p_name, void* p_func, void* p_arg, size_t stSz);

/*thread_util.c*/
extern region_t		getRegionEntry				(uint64_t addr);
extern void			thread_map_init				(void);
extern bool			isValidAddr					(uint64_t addr);
extern bool			isValidPc					(uint64_t pc);
#ifdef __cplusplus
}
#endif

#endif/*__THREADLIB_H__*/
