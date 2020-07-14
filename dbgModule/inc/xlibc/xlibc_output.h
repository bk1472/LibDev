/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_output.h
 *
 *  output api definitions
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */
#ifndef __XLIBC_OUTPUT_H__
#define __XLIBC_OUTPUT_H__
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
#define PRINT_LVL_ALL		2
#define PRINT_LVL_DBG		1
#define PRINT_LVL_NON		0



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
extern int			xlibc_setCoutStat	(int);
extern int			xlibc_setPrintStat	(int);
extern int			xlibc_getPrintStat	(void);

extern int			dbgprint			(const char *format , ... );
extern int			tprint0n			(const char *format , ... );
extern int			tprint1n			(const char *format , ... );
extern int			rprint0n			(const char *format , ... );
extern int			rprint1n			(const char *format , ... );
extern int			eprint0n			(const char *format , ... );
extern int			eprint1n			(const char *format , ... );

extern int			ext_vprintf			(const char* format, va_list ap);
extern void			hexdump				(const char* name, void* vcp, int size);
extern void			hexdump_f			(FILE* stream, const char* name, void* vcp, int size);



#ifdef __cplusplus
}
#endif

#endif/*__XLIBC_OUTPUT_H__*/
