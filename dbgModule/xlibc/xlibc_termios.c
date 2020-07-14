/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_termios.c
 *
 * 	Terminal In/Out Control functions
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
#include	"ext_common.h"
#include	"xlibc.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void xlibc_term_setmode(int flag, int mode)
{
	struct termios termAttr;
	tcgetattr(0, &termAttr);
	termAttr.c_cc[VERASE]  =  0x008; /* Backspace      */
	termAttr.c_lflag      &= ~0xa00; /* Backspace Echo */

	if (mode & ECHO)
	{
		if (flag & ECHO) termAttr.c_lflag |=  ECHO;
		else             termAttr.c_lflag &= ~ECHO;
	}
	if (mode & ICANON)
	{
		if (flag & ICANON) termAttr.c_lflag |=  ICANON;
		else               termAttr.c_lflag &= ~ICANON;
	}

	tcsetattr(0, TCSANOW, &termAttr);
	return;
}

void xlibc_term_setoneshot(TERMINAL_MODE_T tMode)
{
	int flag, mode;

	if     (tMode == SINGLE_KEY_MODE){flag =           0; mode = ICANON|ECHO;}
	else if(tMode == NORMAL_KEY_MODE){flag = ICANON|ECHO; mode = ICANON|ECHO;}
	else                             {flag =      ICANON; mode =           0;}

	xlibc_term_setmode(flag, mode);
}

int xlibc_term_getmode(void)
{
	struct termios termAttr;

	tcgetattr(0, &termAttr);
	return(termAttr.c_lflag);
}

void xlibc_term_setrawmode(int bEnable)
{
	static struct termios	termiosVal;
	static int				bRawMode = 0;

	if (bEnable)
	{
		struct termios buf;
		tcgetattr(0, &termiosVal);
		buf = termiosVal;
		buf.c_lflag &= ~(ECHO | ICANON |IEXTEN | ISIG);
		buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		buf.c_cflag &= ~(CSIZE | PARENB);
		buf.c_cflag |= CS8;
		buf.c_oflag &= ~(OPOST);
		buf.c_cc[VMIN] = 1;
		buf.c_cc[VTIME] = 0;
		tcsetattr(0,TCSAFLUSH,&buf);
		bRawMode = 1;
	}
	else if (!bEnable && bRawMode)
	{
		tcsetattr(0,TCSAFLUSH,&termiosVal);
		bRawMode = 0;
	}
	else
		bRawMode = 0;
}
