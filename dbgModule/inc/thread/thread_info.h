/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file thread_info.h
 *
 *	Thread library hook and extension
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
 *	@note
 *	@see
 */
#ifndef __THREAD_INFO_H__
#define __THREAD_INFO_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define THREAD_NAME_SZ		(10)

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	pthread_t	thId;
	pid_t		tId;
	void*		p_func;
	void*		arg;
} thread_attr_t;

typedef struct _th_info
{
	struct _th_info *next;
	struct _th_info *prev;
	int				tskId;
	char			name[THREAD_NAME_SZ+1];
	thread_attr_t	attr;
	void*			usl;
	size_t			stSz;
} thread_info_t;

typedef struct
{
	uint64_t	reg_beg;
	uint64_t	reg_end;
	uint64_t	reg_opt;
} region_t;

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif/*__THREAD_INFO_H__*/
