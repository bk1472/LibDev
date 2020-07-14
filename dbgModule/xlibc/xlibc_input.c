/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_input.c
 *
 *	Terminal (Keyboard) Input control functions
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 23
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
#include	"cmd_shell.h"
#include	"xlibc.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define SPECIAL_ACT_SIZE		(21)

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	char*	pKey_name;
	int		pc_key;
	void*	pAct_func;
} SP_KEY_ACT_t;

/*-----------------------------------------------------------------------------
 * Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static int			_check_ESC_mode  = 1;
static volatile int _keyPressed      = 0;
static SP_KEY_ACT_t	_spActionTable[SPECIAL_ACT_SIZE] = {
	{"PC_KEY_F01" ,    PC_KEY_F01,     null_function},
	{"PC_KEY_F02" ,    PC_KEY_F02,     null_function},
	{"PC_KEY_F03" ,    PC_KEY_F03,     null_function},
	{"PC_KEY_F04" ,    PC_KEY_F04,     null_function},
	{"PC_KEY_F05" ,    PC_KEY_F05,     null_function},
	{"PC_KEY_F06" ,    PC_KEY_F06,     null_function},
	{"PC_KEY_F07" ,    PC_KEY_F07,     null_function},
	{"PC_KEY_F08" ,    PC_KEY_F08,     null_function},
	{"PC_KEY_F09" ,    PC_KEY_F09,     null_function},
	{"PC_KEY_F10" ,    PC_KEY_F10,     null_function},
	{"PC_KEY_F11" ,    PC_KEY_F11,     null_function},
	{"PC_KEY_F12" ,    PC_KEY_F12,     null_function},
	{"PC_KEY_INS" ,    PC_KEY_INS,     null_function},
	{"PC_KEY_DEL" ,    PC_KEY_DEL,     null_function},
	{"PC_KEY_END" ,    PC_KEY_END,     null_function},
	{"PC_KEY_PGDN",    PC_KEY_PGDN,    null_function},
	{"PC_KEY_BACKQT",  PC_KEY_BACKQT,  null_function},
	{"PC_KEY_TILDE",   PC_KEY_TILDE,   null_function},
	{"PC_KEY_AT",      PC_KEY_AT,      null_function},
	{"PC_KEY_SHARP",   PC_KEY_SHARP,   null_function},
	{"PC_KEY_DOOLLER", PC_KEY_DOOLLER, null_function},
};

static int			_sp_actSort[SPECIAL_ACT_SIZE];

/*-----------------------------------------------------------------------------
 * Static Function Definition
------------------------------------------------------------------------------*/
static int compare_actTbl(const void *a, const void *b)
{
	int cmp = (_spActionTable[*(int*)a].pc_key - _spActionTable[*(int*)b].pc_key);

	return( ( (cmp > 0) ? 1 : ((cmp ==  0) ? 0 : -1) ) );
}

static void __init_action_table(void)
{
	int			n;

	for (n = 0; n < SPECIAL_ACT_SIZE; n++)
	{
		_sp_actSort[n] = n;
	}
	qsort(&_sp_actSort[0], SPECIAL_ACT_SIZE, sizeof(int), compare_actTbl);
}


static int _xlibc_input_process_seq (unsigned char inChar)
{
	static unsigned int	inCode = 0;
	static unsigned int	kLinux = 0;
	static signed int	index  = 0;

	uint32_t			key    = 0;

	index++;

	if (inChar == 0x1B) inCode = 0;
	else                inCode = (inCode << 8) | inChar;

	/* Append tailing ~ for VT Linux key input sequence */
	if      ((index == 3) && (inChar == 0x5B)) { kLinux = 1; inChar = 0;    }
	else if ((index == 4) && (kLinux ==    1)) { kLinux = 0; inChar = 0x7E; }

	if      ((index == 1) && (inChar != 0x1B)) key = inChar;
	else if ((index == 2) && (inChar != 0x5B)) key = inChar;
	else if ((index == 3) && (inChar  > 0x40)) key = inCode;
	else if ((index == 4) && (inChar == 0x7E)) key = inCode;
	else if ((index == 5) && (inChar == 0x7E)) key = inCode;
	else if ( index >  5)                      kLinux = index = inCode = 0;

	if (key)
	{
		index  = 0;
		inCode = 0;
	}
	return(key);
}

#if   defined (__UCLIBC__)
#define IF_INKEY()	if (fp->__bufpos != fp->__bufread)
#elif defined (__GLIBC__)
#define IF_INKEY()	if (fp->_IO_read_ptr != fp->_IO_read_end)
#else
#error Escape sequence processor should be ported
#endif
static int _xlibc_input_check (FILE *fp, int prevCount, int waitMs)
{
	int		rc = 0;

	if(0 == _check_ESC_mode)
		return 1;

	xlibc_term_setmode(ICANON, ICANON);	// stty icanon
	IF_INKEY()
	{
		rc = 1;
	}
	xlibc_term_setmode(0     , ICANON);	// stty -icanon

	return(rc);
}

static int __get_unified_special_key(int in_key)
{
	switch (in_key)
	{
		case PC_KEY_F01: case PC_KEY_CRT_SANSI_F01: case PC_KEY_CRT_LINUX_F01:
			in_key = PC_KEY_F01; break;
		case PC_KEY_F02: case PC_KEY_CRT_SANSI_F02: case PC_KEY_CRT_LINUX_F02:
			in_key = PC_KEY_F02; break;
		case PC_KEY_F03: case PC_KEY_CRT_SANSI_F03: case PC_KEY_CRT_LINUX_F03:
			in_key = PC_KEY_F03; break;
		case PC_KEY_F04: case PC_KEY_CRT_SANSI_F04: case PC_KEY_CRT_LINUX_F04:
			in_key = PC_KEY_F04; break;
		case PC_KEY_F05: case PC_KEY_CRT_SANSI_F05: case PC_KEY_CRT_LINUX_F05:
			in_key = PC_KEY_F05; break;
		case PC_KEY_F06: case PC_KEY_CRT_SANSI_F06:
			in_key = PC_KEY_F06; break;
		case PC_KEY_F07: case PC_KEY_CRT_SANSI_F07:
			in_key = PC_KEY_F07; break;
		case PC_KEY_F08: case PC_KEY_CRT_SANSI_F08:
			in_key = PC_KEY_F08; break;
		case PC_KEY_F09: case PC_KEY_CRT_SANSI_F09:
			in_key = PC_KEY_F09; break;
		case PC_KEY_F10: case PC_KEY_CRT_SANSI_F10:
			in_key = PC_KEY_F10; break;
		case PC_KEY_F11: case PC_KEY_CRT_SANSI_F11:
			in_key = PC_KEY_F11; break;
		case PC_KEY_F12: case PC_KEY_CRT_SANSI_F12:
			in_key = PC_KEY_F12; break;
		case PC_KEY_INS: case PC_KEY_CRT_SANSI_HOME:
			in_key = PC_KEY_INS; break;
		case PC_KEY_DEL: case PC_KEY_CRT_SANSI_END:
			in_key = PC_KEY_DEL; break;
		case PC_KEY_END: case PC_KEY_CRT_SANSI_PGUP:
			in_key = PC_KEY_END; break;
		case PC_KEY_PGDN: case PC_KEY_CRT_SANSI_PGDN:
			in_key = PC_KEY_PGDN; break;

	};

	return in_key;
}

static SP_KEY_ACT_t* __get_action_item(int inKey)
{
	static int	_init = 0;
	int			x, l = 0, r = SPECIAL_ACT_SIZE -1;
	int			matched = 0;

	if(!_init)
	{
		__init_action_table();
		_init = 1;
	}

	do
	{
		x = (l + r)/2;
		if      (inKey  < _spActionTable[_sp_actSort[x]].pc_key) {matched = 0; r = x - 1;}
		else if (inKey == _spActionTable[_sp_actSort[x]].pc_key) {matched = 1; l = x + 1;}
		else                                                     {matched = 0; l = x + 1;}
	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		return &(_spActionTable[_sp_actSort[x]]);
	}

	return NULL;
}

static int _check_special_key(int *pInKey)
{
	typedef int (*ACT_FUNC_t)(int*);
	bool			processed = true;
	int				rc;
	SP_KEY_ACT_t*	pAct      = NULL;
	ACT_FUNC_t 		func      = NULL;

	switch(*pInKey)
	{
		case '`': *pInKey = PC_KEY_BACKQT;  break;
		case '~': *pInKey = PC_KEY_TILDE;   break;
		case '@': *pInKey = PC_KEY_AT;      break;
 		case '#': *pInKey = PC_KEY_SHARP;   break;
 		case '$': *pInKey = PC_KEY_DOOLLER; break;
		default : *pInKey = __get_unified_special_key(*pInKey);
	}

	pAct = __get_action_item(*pInKey);

	if (NULL == pAct)
	{
		processed = false;
		goto _END;
	}

	func = (ACT_FUNC_t)pAct->pAct_func;
	rc = func(pInKey);

	if (rc == 0) processed = true;
	else         processed = false;

_END:
	return processed;
}

static int _input_get_keypressed(void)
{
	return _keyPressed;
}

/*-----------------------------------------------------------------------------
 * API Function Definition
------------------------------------------------------------------------------*/
void* xlibc_reg_cmdkey(int inKey, void* pNewAct)
{
	SP_KEY_ACT_t*	p_act = __get_action_item(inKey);
	void*			oldFunc;

	if(p_act == NULL)
		return NULL;

	oldFunc = p_act->pAct_func;
	p_act->pAct_func = pNewAct;

	return oldFunc;
}

#include	<sys/poll.h>
int xlibc_rgetc_t(FILE *fp, int waitMs)
{
	struct pollfd	pfd;
	int				ch = 0;
	int				prevMode = xlibc_term_getmode();

	if (fp == NULL)
		fp = stdin ;

	xlibc_term_setmode(0     , ICANON);	// stty -icanon
	pfd.fd		= 0;
	pfd.events	= POLLIN | POLLPRI;
	pfd.revents	= 0;
	if (poll(&pfd, 1, waitMs) > 0)
	{
		ch = getc(fp);
	}
	xlibc_term_setmode(prevMode, ICANON | ECHO);

	return(ch);
}

int xlibc_rgetc(FILE *fp)
{
	int		inKey;

	if (fp == NULL)
		fp = stdin ;

	/*	Save terminal Mode	*/
	xlibc_term_getmode();

	xlibc_term_setrawmode(1);

	inKey = getc(fp);

	/*	Restore terminal Mode	*/
	xlibc_term_setrawmode(0);

	return(inKey);
}

int xlibc_getc(FILE *fp)
{
	int	inKey, prevKeyPressed;
	int	prevMode;

	if (fp == NULL)
		fp = stdin ;

	/*	Save terminal Mode	*/
	prevMode = xlibc_term_getmode();

RescanInput:
	prevKeyPressed = _input_get_keypressed() + 1;

	xlibc_term_setmode(0     , ICANON);	// stty -icanon
	inKey = getc(fp);

	if (inKey == PC_KEY_ESC)
	{
		/*	Check for escape sequnce by waiting	24ms	*/
		/* next characters to be entered				*/
		if (_xlibc_input_check(fp, prevKeyPressed, 24))
		{
			int	inKey2 = inKey;

			/* Escape sequence entered, like page up	*/
			while (1)
			{
				inKey = _xlibc_input_process_seq(inKey2);
				if (inKey != 0)
					break;
				inKey2 = getc(fp);
			}
			xlibc_term_setmode(ICANON, ICANON);	// stty icanon
		}
	}
	else
	{
		inKey = _xlibc_input_process_seq((unsigned char)inKey);
	}

	if (       (inKey >= 0x100)
			|| (inKey ==   '`')
			|| (inKey ==   '~')
			|| (inKey ==   '@')
 		    || (inKey ==   '#')
 		    || (inKey ==   '$')
	   )
	{
		if (_check_special_key(&inKey))
			goto RescanInput;
	}

	/*	Restore terminal Mode	*/
	xlibc_term_setmode(prevMode, ICANON );

	return(inKey);
}

int xlibc_getc_e(FILE *fp)
{
	int ch = xlibc_getc(fp);

	putchar(ch);
	return(ch);
}

char* xlibc_gets(char* buf, int n, FILE* fp)
{
	int		key, i;

	if((buf == NULL) || (n < 0))
		return NULL;

	if(fp == NULL)
		fp = stdin;

	fflush(fp);

	while(1)
	{
		i = 0;

		key = xlibc_getc(fp);

		if(key >= PC_KEY_CRT_SANSI_UP && key <= PC_KEY_CRT_SANSI_LEFT)
		{
			return buf;
		}

		if (key != '\n')
		{
			ungetc(key, fp);
		}

		if(xlibc_term_getmode() & ECHO)
			xlibc_term_setmode(ECHO  , ICANON | ECHO);	// stty echo -icanon
		else
			xlibc_term_setmode(0     , ICANON | ECHO);	// stty echo off -icanon

		for( ; (key != '\n') && (i < (n-1)) ; )
		{
			key = getc(fp);
			if (key == 0x7f) //backspace
			{
				putchar('\b');
				putchar(' ');
				if (i > 0)
				{
					i--;
					putchar('\b');
				}
			}
			else if(key == '\n')
			{
				buf[i++] = key;
				break;
			}
			else
			{
				buf[i++] = key;

				if(i >= n-1) putchar('\n');
			}
			fflush(stdout);
		}
		buf[i] = '\0';
		xlibc_term_setmode(ICANON, ICANON);	// stty icanon

		return (buf);
	}
}
