/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_util.h
 *
 *	System utility library file
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */
#ifndef __XLIBC_UTIL_H__
#define __XLIBC_UTIL_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"

#include	<sys/time.h>
#include	<time.h>
#include	<ctype.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define NO_WAIT								( 0)
#define WAIT_FOREVER						(-1)

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * External function Declarations
------------------------------------------------------------------------------*/
extern struct timespec	xlibc_ms2abstime		(int);
extern uint64_t			xlibc_read_usticks		(void);
extern uint64_t			xlibc_read_msticks		(void);
extern void				xlibc_delay_us			(unsigned int);
extern void				xlibc_delay_ms			(unsigned int);
extern unsigned long	xlibc_suspend			(unsigned int);


/*string operationn extension*/
extern unsigned long	strtoul2				(const char*, char**, int);
extern int				str2argv				(char*, int, char**);
extern int				str2indexInOpts			(char**, const char*, int*);
extern char*			strtok2					(char *srcStr, const char *del, char **ptr);
extern char*			strtrim					(char*);

#ifdef __cplusplus
}
#endif

#endif/*__XLIBC_UTIL_H__*/
