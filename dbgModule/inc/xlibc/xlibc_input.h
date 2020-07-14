/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_input.h
 *
 *  input api definitions
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */
#ifndef __XLIBC_INPUT_H__
#define __XLIBC_INPUT_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc_termios.h"
#include	"pc_key.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
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

/*-----------------------------------------------------------------------------
 * External function Declarations
------------------------------------------------------------------------------*/
extern void*	xlibc_reg_cmdkey			(int inKey, void* pNewAct);
extern int		xlibc_rgetc_t				(FILE *fp, int waitMs);
extern int		xlibc_rgetc					(FILE *fp);
extern int		xlibc_getc					(FILE *fp);
extern int		xlibc_getc_e				(FILE *fp);
extern char*	xlibc_gets					(char* buf, int n, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif/*__XLIBC_INPUT_H__*/
