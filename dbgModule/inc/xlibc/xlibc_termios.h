/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_termios.h
 *
 *	Terminal In/Out mode control
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
 *	@note
 *	@see
 */
#ifndef __XLIBC_TERMIOS_H__
#define __XLIBC_TERMIOS_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	<termios.h>
#include	<unistd.h>

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
typedef enum {
	SINGLE_KEY_MODE,
	NORMAL_KEY_MODE,
	NO_ECHO_MODE
} TERMINAL_MODE_T;

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * External Used Functions Declaration
------------------------------------------------------------------------------*/
extern  void	xlibc_term_setmode			(int flag, int mode);
extern  void	xlibc_term_setoneshot		(TERMINAL_MODE_T tMode);
extern  int 	xlibc_term_getmode			(void);
extern  void	xlibc_term_setrawmode		(int bEnable);

#ifdef __cplusplus
}
#endif

#endif/*__XLIBC_TERMIOS_H__*/
