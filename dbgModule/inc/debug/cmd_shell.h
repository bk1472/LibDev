/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file cmd_shell.h
 *
 *	command shell types and APIs
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
 *	@note
 *	@see
 */
#ifndef __CMD_SHELL_H__
#define __CMD_SHELL_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc_util.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define	DMT_NONE		 0		/* Pass no arguement       				*/
#define	DMT_ARG1		11		/* Pass only first one arguement as str */
#define	DMT_ARG2		12		/* Pass only first two arguement as str */
#define	DMT_ARGV		18		/* Pass argc and argv list  			*/
#define	DMT_ARGCV		19		/* Pass argv list        				*/
#define	DMT_OPTS		21		/* Pass optional arguement 				*/
#define DMT_INDX		22		/* Pass Index of matched options string	*/
#define DMT_LINK		23		/* link to command in other menu 		*/
#define	DMT_DESC		80		/* Item describing current menu			*/
#define	DMT_EXTM		81		/* Link to user added command menus		*/
#define	DMT_TERM		82		/* End mark of list of menu items		*/
#define	DMT_MENU		90		/* Pass Menu Struct & Argv List 		*/
								/* This should be last item				*/

#define	DM_SHOW_HELP	 1		/* Show help on entering new menu		*/
#define	DM_EXIT_ON_CR	 2		/* Exit on enter command				*/
#define DM_BUF_ALLOC	 4		/* CMD_DBG_MENU_T stroage is allocated on	*/


/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define	TAG_USAGE_LABEL(expr)						\
	if (expr)										\
	{												\
PrintHelpAndReturn:									\
		cmdHelpMenu(NULL, 0);						\
		return;										\
	}

#define	GOTO_USAGE_LABEL()	goto PrintHelpAndReturn

#define DMT_HELP_TAG		"^"
#define DMT_VOPT_END		"\0"

#define	GET_EXTM(_p)		((CMD_DBG_MENU_T *)_p->dm_next - 1)

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef	void	VFP_t(void);
typedef void	UFP_t(uint32_t);

/**
 * Debug Menu를 위한 구조체 초기값
 * 	- 첫번째 Entry는 Prompt를 위해 Description만 사용한다.
 * 	- 마지막 Entry는 termination을 위해 dm_type이 반드시
 * 		DMT_TERM(Terminal mode 인 경우) 또는
 * 		DMT_EXTM(Extended menu 가 있는 경우)
 * 	  이어야 한다. DM_TERM일 때는 dm_name도 같이 NULL이어야 한다.
 *
 *	- dm_type 정의 방법.
 *	  DMT_NONE : 아무런 Argument도 전달하지 않는다.
 *	  	f(void)
 *
 *	  DMT_ARG1:	명령행에서 입력한 argument를 첫번째 것만 String으로 전달
 *	  	f(char *arg1)
 *
 *	  DMT_ARG2:	명령행에서 입력한 argument를 처음 두개만 String으로 전달
 *	  	f(char	*arg1, char	*arg2)
 *
 *	  DMT_OPTS:	dm_opts에 정의된 값을 argument로 전달한다.
 *	  	f(int)
 *
 *	  DMT_INDX: dm_help에 정의된 option string과 입력된 argument를 비교하여
 *	  			일치하는 option의 번호를 argument로 전달한다.
 *		f(int)
 *		[1..15]			: 1부터 15까지의 숫자. 입력하지 않으면 -1.
 *		1..15			: 1부터 15까지의 숫자. 입력하지 않으면 오류.
 *		[off|on|str]	: "on" = 0, "off" = 1, "str" = 2, 입력하지 않으면 -1
 *		off|on|str		: "on" = 0, "off" = 1, "str" = 2, 입력하지 않으면 오류.
 *
 *	  DMT_ARGV:	명령행에서 입력한 argument룰 argvlist 로 만들어서 전달한다.
 *	  	f(char	**argv)
 *
 *	  DMT_ARGCV: 명령행에서 입력한 argument를 argvlist로 만들어서 전달한다.
 *	  	f(int argc, char	**argv)
 *
 *	  DMT_LINK:	다른 메뉴 구조체의 명령을 실행한다. 실행할 명령은
 *	  			dm_help에 기록한다.
 *
 *	  DMT_MENU:	dm_args 에 정의된 Menu 구조체와 argvlist를 함께전달한다.
 *	  	f(CMD_DBG_MENU_T*, char	**argv)
 *
 *	  DMT_DESC:	Menu list중 제일 처음 것으로 Menu의 내용에 대한 설명을 가진다.
 *
 *	  DMT_EXTM:	Menu list의 제일 마지막 것으로 dm_next가 가리키는 곳에 확장
 *	  			menu(사용자가 추가 등록한 것들)가 있음을 알린다.
 *
 *	  DMT_TERM:	Menu list의 제일 마지막 것으로 list의 끝을 알린다.
 *	  			dm_name도 같이 NULL이어야 한다.
 */
typedef struct {
	char		*dm_name;		/* Command Name of debug menu			*/
	void		*dm_next;		/* Call Context for Next Level			*/
								/*  1. Function pointer to be called	*/
								/*	2. Menu Structure to be passed		*/
	int			dm_opts;		/* Optional argument to be passed		*/
	int			dm_type;		/* Call type of sub function or Menu	*/
	char		*dm_desc;		/* Description of debug menu			*/
	char		*dm_help;		/* Detail help message of usage 		*/
								/*  must start with option strings, it	*/
								/*  will be parsed while command line	*/
								/*  interpreting 						*/
} CMD_DBG_MENU_T;

/*-----------------------------------------------------------------------------
 * External Used variables and Functions
------------------------------------------------------------------------------*/
/* cmd_shell.c */
extern void			cmd_setDebugLevel		(int);
extern int			cmd_getDebugLevel		(void);
extern bool			cmd_setDebugMode		(bool);
extern bool			cmd_getDebugMode		(void);
extern void			cmd_setExcMode			(bool);
extern void			cmdHelpMenu				(CMD_DBG_MENU_T*, int);
extern void			cmdDebugMain			(void);
extern void			init_debug_system		(void);

/* cmd_input.c */
extern uint32_t		cmd_getIntegerInput		(const char*, int32_t*);
extern uint32_t		cmd_getHexInput			(const char*, uint32_t*);
extern uint32_t		cmd_getDecimalInput		(const char*, uint32_t*);
extern uint32_t		cmd_getDoubleInput		(const char*, double*);
extern uint32_t		cmd_getFloatInput		(const char*, float*);
extern uint32_t		cmd_readCmdString		(const char*, char*, size_t);
extern uint32_t		cmd_readNumber			(const char*, int, uint32_t);

#ifdef __cplusplus
}
#endif

#endif/*__CMD_SHELL_H__*/
