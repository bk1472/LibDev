/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file cmd_shell.c
 *
 *	Robot system command shell libraries
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
#include	"pc_key.h"
#include	<ctype.h>
#include	"dsm.h"
#include	"xlibc.h"
#include	"cmd_shell.h"
#include	"thread.h"
#include	"symbol.h"

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
static bool				_debugEnable		= true;
static int				_debugLevel			= 0;
static CMD_DBG_MENU_T*	_pCurrMenu			= NULL;
static char*			_pCurrCmd			= NULL;
static char*			_EnableMsgs[]		= {"Disable", "Enable"};
static bool				_excMode            = false;



static CMD_DBG_MENU_T	_SDM_Global[];
static CMD_DBG_MENU_T	_SDM_Test[];
static CMD_DBG_MENU_T	_SDM_Main[];

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static char *_getDesc(CMD_DBG_MENU_T *pCurrMenu)
{
	CMD_DBG_MENU_T	*pLinkMenu;
	static char	descStr[128];

	if (pCurrMenu->dm_type == DMT_LINK)
	{
		pLinkMenu = pCurrMenu->dm_next;
		snprintf(descStr, 80, "%-24s ---> %s@%s", pCurrMenu->dm_desc, pCurrMenu->dm_help, pLinkMenu->dm_desc);
	}
	else
	{
		if(pCurrMenu->dm_desc) snprintf(descStr, 80, "%s", pCurrMenu->dm_desc);
		else                   descStr[0] = 0;
	}

	return(&descStr[0]);
}


static void _helpMenu(CMD_DBG_MENU_T *pMenu, int help_mode)
{
	int			len;
	char		*cp1, *cp2;
	char		typeChar = ' ';
	CMD_DBG_MENU_T	*pCurrMenu;

	if (help_mode == 0)
	{
		if (pMenu)
			pCurrMenu = pMenu;
		else
			pCurrMenu = _pCurrMenu;

		if ((pCurrMenu->dm_help == NULL) || (pCurrMenu->dm_type == DMT_LINK))
		{
			if (pCurrMenu->dm_type == DMT_LINK) typeChar = '>';
			else                                typeChar = ' ';

			rprint1n("  %c %-8.8s %s", typeChar, _pCurrCmd,  _getDesc(pCurrMenu));
		}
		else
		{
			rprint1n("    %s", pCurrMenu->dm_desc);
			cp1 = pCurrMenu->dm_help;
			cp2 = strchr(cp1, DMT_HELP_TAG[0]);
			if (cp2) len = cp2 - cp1;
			else     len = strlen(cp1);

			rprint1n("    Usage: %s %*.*s", _pCurrCmd, len, len, cp1);
			cp1 += (len+1);

			while (*cp1 && (cp2))
			{
				cp2 = strchr(cp1, DMT_HELP_TAG[0]);
				if (cp2) len = cp2 - cp1;
				else     len = strlen(cp1);

				rprint1n("\t %*.*s", len, len, cp1);

				cp1 += (len+1);
			}
		}
	}
	else if ( (help_mode == 1) || (help_mode == 2) )
	{
		int			i;
		CMD_DBG_MENU_T *pMenuList[2], *pParMenu;

		rprint1n("    %-8.8s Print this help message", "help,?");
		rprint1n("    ============================================================");

		pMenuList[0] = &_SDM_Global[1];
		pMenuList[1] = pMenu+1;

		for (i = 0; i < 2; i++)
		{
			if (i > 0)
			{
				pCurrMenu = pMenuList[i] - 1;
				rprint1n("    ------ %-12s ----------------------------------------", pCurrMenu->dm_desc);
			}

			for (pCurrMenu = pMenuList[i]; pCurrMenu->dm_type != DMT_TERM; pCurrMenu++)
			{
				int		dm_type  = pCurrMenu->dm_type;
				char	*dm_name = pCurrMenu->dm_name;

				if      (dm_type == DMT_EXTM) { pCurrMenu = GET_EXTM(pCurrMenu); continue; }
				else if (dm_type == DMT_LINK) typeChar = '>';
				else if (dm_type == DMT_MENU) typeChar = '*';
				else                          typeChar = ' ';

				rprint1n("  %c %-8.8s %s", typeChar, dm_name, _getDesc(pCurrMenu));
			}
		}

		if (help_mode == 2)
		{
			for (pParMenu = pMenuList[1]; pParMenu->dm_type != DMT_TERM; pParMenu++)
			{
				if (pParMenu->dm_type == DMT_EXTM)
				{
			    	pParMenu = GET_EXTM(pParMenu);
					continue;
				}

				if (pParMenu->dm_type != DMT_MENU)
					continue;

				rprint1n("    ----- [SubMenu]::%-12s ------------------------------", pParMenu->dm_name);

				pCurrMenu = (CMD_DBG_MENU_T *)pParMenu->dm_next + 1;
				for (; pCurrMenu->dm_type != DMT_TERM; pCurrMenu++)
				{
					int 	dm_type  = pCurrMenu->dm_type;
					char	*dm_name = pCurrMenu->dm_name;

					if      (dm_type == DMT_EXTM) { pCurrMenu = GET_EXTM(pCurrMenu); continue; }
					else if (dm_type == DMT_LINK) typeChar = '>';
					else if (dm_type == DMT_MENU) typeChar = '*';
					else                          typeChar = ' ';

					rprint1n("  %c %-8.8s %s", typeChar, dm_name, _getDesc(pCurrMenu));
				}
			}
		}

		rprint1n("    ============================================================");
		rprint1n("    %-8.8s Exit from %s menu\n", "exit", pMenu->dm_desc);
	}
	return;
}


static int _mainMenu(CMD_DBG_MENU_T *pMenu, char **argv)
{
	#define MAX_MENU_DEPTH		64
	#define NUM_HISTORY			10
	#define CMD_STR_LEN			80

	static CMD_DBG_MENU_T	*ppMenuStack[MAX_MENU_DEPTH] = {NULL, };
	CMD_DBG_MENU_T			*pCurrMenu;
	static char				cmd_hist[NUM_HISTORY][CMD_STR_LEN];
	int						argc, value;
	static bool				verbose   = false;
	static int				curr_hist = NUM_HISTORY -1;
	static int				last_hist = NUM_HISTORY -1;
	bool					do_loop   = true;
	bool					error_flag;
	int						cmd_type  = 0;
	int						newMenuLevel = -1, popLevel = 0;
	char					cmdStr[CMD_STR_LEN];
	char					*cmd, **new_argv, *argList[16];
	bool					saveDebugEnable = _debugEnable;


	if (NULL == pMenu)
		pMenu = &_SDM_Main[0];

	argc = 0;
	if((argv != NULL) && (*argv != NULL))
	{
		do_loop = false;
		for(argc = 0; argv[argc] != 0; argc++)
			;
	}

	if (_debugLevel == 0)
	{
		if(pMenu->dm_opts & DM_SHOW_HELP)
			_debugEnable = true;
	}

	/* Push the pointer to current menu			*/
	ppMenuStack[_debugLevel] = pMenu;

	/* Increase Menu Level						*/
	_debugLevel++;

	/*	Loop to read command and excute it		*/
	do
	{
		char			*dm_name;
		char			*dm_help;
		int				dm_type;
		int				dm_opts;
		int				cHistory;
		int				repeatMs = 0;
		void			(*dm_func)(void);
		CMD_DBG_MENU_T	*dm_menu;

		cHistory = 0;
		repeatMs = 0;

		newMenuLevel = _debugLevel - 1;

		if(do_loop)
		{
			if(_debugLevel == 0)
				goto exit_menu;

			if (pMenu->dm_opts & DM_SHOW_HELP)
				_helpMenu(pMenu, 1);

			if(_excMode == true)
				rprint0n("^r^\n%d:%s[exc]", _debugLevel, pMenu->dm_desc);
			else
				rprint0n("\n%d:%s", _debugLevel, pMenu->dm_desc);
			(void)cmd_readCmdString(" $ ", &cmdStr[0], CMD_STR_LEN);
			/*	Save command string for history	*/
			if ( cmdStr[0] != '!')
				strcpy(cmd_hist[0], cmdStr);

ParseHistory:
			/*	Skip heading white spaces		*/
			cmd = &cmdStr[0];
			while ((*cmd == ' ') || (*cmd == '\t'))
				cmd++;
			/*	Check if referencing from root	*/
			if (*cmd == '/')
				newMenuLevel = 0;
			/*	Convert '/' to space			*/
			while ((*cmd != ' ') && (*cmd != '\t') && (*cmd != '\0'))
			{
				if (*cmd == '/') *cmd = ' ';
				cmd++;
			}

			/*	Tokenize as command and args	*/
			argv = argList;
			memset(argList, 0, sizeof(argList));
			argc = str2argv(&cmdStr[0], 8, argv);
			if (verbose)
			{
				rprint1n("+%d:: argc = %d, argv = %x/%x/%x",
					_debugLevel, argc, argv[0], argv[1], argv[2]);
			}
		}

		while ((argc > 0) && (strcmp("..", argv[0]) == 0))
		{
			argc--;
			argv++;
			if (newMenuLevel > 0) newMenuLevel--;
		}

		if (argc == 0)
		{
			if ((argc == 0) && (_debugLevel > 0) && (pMenu->dm_opts & DM_EXIT_ON_CR))
			{
				popLevel = 1;
				break;
			}
			if (_debugLevel > (newMenuLevel + 1))
				popLevel = (_debugLevel - (newMenuLevel + 1));
			continue;
		}

		cmd = argv[0];
		if (cmd[0] == '!')
		{
			if ( (cmd[1] >= '1') && (cmd[1] <= '9') )
			{
				last_hist = cmd[1] - '0';
			}
			else if (cmd[1] != 0)
			{
				int	i, idx, len;

				len = strlen(cmd+1);
				idx = last_hist;
				for (i = 0; i < NUM_HISTORY - 1; i++, idx--)
				{
					if (idx == 0)
						idx = NUM_HISTORY -1;
					if (strncmp(cmd+1, cmd_hist[idx], len) == 0)
					{
						last_hist = idx;
						break;
					}
				}
			}

			if ( (argc > 1) && (argv[1] != NULL) )
			{
				repeatMs = (int)strtoul2(argv[1], NULL, 0);
			}

			cHistory = last_hist;
			strcpy(cmdStr, cmd_hist[cHistory]);
			if (verbose)
			{
				rprint1n("+%d:: cHist = %d, cmd = '%16.16s'",
						_debugLevel, cHistory, cmd_hist[cHistory]);
			}

			goto ParseHistory;
		}
		/*               cmd_type     0         1            2    3           */
		(void) str2indexInOpts(&cmd, "help,?,ls|exit,x,,ff|hist|verbose,verb", &cmd_type);

		/*	Check type of command				*/
		if		(cmd_type == 0) { cmd = argv[1];		/* Help				*/	}
		else if (cmd_type == 1) { popLevel = 1; break;	/* Return to parent */	}
		else if (cmd_type == 2)
		{
			int	i;

			for (i = 1; i < NUM_HISTORY; i++)
			{
				rprint1n("  %d: %s", i, cmd_hist[i]);
			}
			continue;
		}
		else if (cmd_type == 3)
		{
			verbose = !verbose;
			rprint1n("Turn '%s' verbose flag", (verbose ? "on " : "off"));
			continue;
		}
		else if (cHistory == 0 && cmdStr[0] != '!')
		{
			int	i;

			for (i = 1; i < NUM_HISTORY; i++)
			{
				if (strcmp(cmd_hist[0], cmd_hist[i]) == 0)
					break;
			}

			if (i == NUM_HISTORY)
			{
				curr_hist = (curr_hist % 9) + 1;
				last_hist = curr_hist;
				strcpy(cmd_hist[curr_hist], cmd_hist[0]);
				if (verbose)
				{
					rprint1n("+%d:: Hist last %d curr %d %s %s",
							_debugLevel, last_hist, curr_hist, cmd_hist[curr_hist], cmd_hist[0]);
				}
			}
		}

		if (cmd == NULL)
			cmd = (char *)"(void)";
		pCurrMenu = &_SDM_Global[1];
		dm_name	  = (char *)"None";

		if (cmd_type != 1)		/* Not ! cmd	*/
		{
			int			i, depth = 0;
			CMD_DBG_MENU_T *pMenuList[2];

			pMenuList[0] = &_SDM_Global[1];
			pMenuList[1] = ppMenuStack[newMenuLevel]+1;

			for (i = 0; i < 2; i++)
			{
				for (pCurrMenu = pMenuList[i]; (dm_name = pCurrMenu->dm_name) != NULL; pCurrMenu++)
				{
					int	dummy;

					if (pCurrMenu->dm_type == DMT_EXTM)
					{
						pCurrMenu = GET_EXTM(pCurrMenu);
						continue;
					}
//					if (strcasecmp(dm_name, cmd) == 0)
					if (str2indexInOpts(&cmd, dm_name, &dummy) >= 0)
					{
						i = 99;	/* Force to exit outer for loop */
						break;
					}
				}
			}

			_pCurrCmd = cmd;	/* Save Real Command Name	*/

			while  ( (pCurrMenu->dm_type == DMT_LINK) && (pCurrMenu->dm_help != NULL))
			{
				cmd       = pCurrMenu->dm_help;
				pCurrMenu = (CMD_DBG_MENU_T *)pCurrMenu->dm_next + 1;
				for (; (dm_name = pCurrMenu->dm_name) != NULL; pCurrMenu++)
				{
					if (pCurrMenu->dm_type == DMT_EXTM)
					{
						pCurrMenu = GET_EXTM(pCurrMenu);
						continue;
					}
					if (strcasecmp(dm_name, cmd) == 0)
						break;
				}

				/* Prevent from Recursive Linking	*/
				if (++depth > 5) break;
			}
		}

		dm_type = pCurrMenu->dm_type;
		dm_opts = pCurrMenu->dm_opts;
		dm_help = pCurrMenu->dm_help;
		dm_func = (VFP_t       *)pCurrMenu->dm_next;
		dm_menu = (CMD_DBG_MENU_T *)pCurrMenu->dm_next;

		if (cmd_type == 0)		/* Help		*/
		{
			if (argc <= 1)
			{
				_helpMenu(ppMenuStack[newMenuLevel], 1);		/* overall help message		*/
			}
			else if (dm_type < DMT_MENU)
			{
				if (dm_name != NULL)
				{
					_pCurrMenu = pCurrMenu;
					_helpMenu(pCurrMenu, 0);	/* Detail help message		*/
				}
				else if (strcmp(cmd, "*") == 0)
				{
					_helpMenu(ppMenuStack[newMenuLevel], 2);
				}
				else
				{
					rprint1n("'%s' is not in command list", cmd);
				}
			}
			else
			{
				/*
				**	Cascade to next menu level
				**	Command line was entered as "help video", so we must
				**	send "help" as initial command of video menu.
				*/
				argv[1] = argv[0];
				_mainMenu(dm_menu, argv+1);
			}
		}
		else
		{
			/*	No matched command found		*/
			if (dm_name == NULL)
			{
				rprint1n("Command not found '%s'", cmd);
				continue;
			}

RepeatCommand:
			if (repeatMs)
				rprint1n("^B^Repeating command '%s' every %d ms, Press <ESC> to quit", cmdStr, repeatMs);

			_pCurrMenu = pCurrMenu;
			error_flag = true;
			new_argv   = ((argc > 1) ? (argv+1) : NULL);

			switch (dm_type)
			{
			  case DMT_NONE:
				if (verbose)
					rprint1n("+%d:: Type %d cmd : %s()", _debugLevel, dm_type, dm_name);
				dm_func();
				error_flag = false;
				break;
			  case DMT_ARG1:
				if (argc == 2)
				{
					if (verbose)
						rprint1n("+%d:: Type %d cmd : %s(%s)", _debugLevel, dm_type, dm_name, argv[1]);
					((void (*)(char *))dm_func)(argv[1]);
					error_flag = false;
				}
				break;
			  case DMT_ARG2:
				if (argc == 3)
				{
					if (verbose)
					{
						rprint1n("+%d:: Type %d cmd : %s(%s,%s)",
							_debugLevel, dm_type, dm_name, argv[1], argv[2]);
					}
					((void (*)(char *, char *))dm_func)(argv[1], argv[2]);
				}
				break;
			  case DMT_OPTS:
				if (verbose)
					rprint1n("+%d:: Type %d cmd : %s(%d)", _debugLevel, dm_type, dm_name, dm_opts);
				((void (*)(int))dm_func)(dm_opts);
				error_flag = false;
				break;
			  case DMT_INDX:
				if (str2indexInOpts(new_argv, dm_help, &value) >= 0)
				{
					if (verbose)
						rprint1n("+%d:: Type %d cmd : %s(%d)", _debugLevel, dm_type, dm_name, value);
					((void (*)(int))dm_func)(value);
					error_flag = false;
				}
				break;
			  case DMT_ARGV:
				if (verbose)
					rprint1n("+%d:: Type %d cmd : %s(0x%x)", _debugLevel, dm_type, dm_name, new_argv);
				((void (*)(char **))dm_func)(new_argv);
				error_flag = false;
				break;
			  case DMT_ARGCV:
				if (verbose)
					rprint1n("+%d:: Type %d cmd : %s(0x%x)", _debugLevel, dm_type, dm_name, new_argv);
				((void (*)(int, char **))dm_func)(argc - 1, new_argv);
				error_flag = false;
				break;
			  case DMT_MENU:
				if (verbose)
					rprint1n("+%d:: Enter Menu : %s(0x%x)", _debugLevel, dm_name, new_argv);
				if (_debugLevel < MAX_MENU_DEPTH)
				{
					popLevel = _mainMenu(dm_menu, new_argv);
				}
				else
				{
					rprint1n("Can not enter into new menu\n");
				}
				error_flag = false;
				break;
			  default:
				/* Just Ignore Unknown type of command */
				break;
			}

			if (error_flag)
			{
				_helpMenu(NULL, 0);
			}

			if (repeatMs > 0)
			{
				int	ch = xlibc_rgetc_t(stdin, repeatMs);
				if (ch != 0x1b)
					goto RepeatCommand;
			}
		}
	} while (do_loop && (popLevel == 0));

	/* Decrease Menu Level						*/
	_debugLevel--;

exit_menu:
	if (_debugLevel == 0)
	{
		_debugEnable = saveDebugEnable;
	}

	return ( (popLevel>0) ? popLevel-1 : 0 );
}

/*-----------------------------------------------------------------------------
 *	debug command Function Definition
------------------------------------------------------------------------------*/
#ifndef __CMD_SHELL_C__
#define __CMD_SHELL_C__
#include	"./cmd_func.c"
#endif/*__CMD_SHELL_C__*/

/*-----------------------------------------------------------------------------
 *	special key control function registering functions
------------------------------------------------------------------------------*/
static int _workF08(int* pInKey)
{
	static char* strStat[] = {"Non", "Dbg", "All"};
	int rc = 0;

	int print_lvl = xlibc_getPrintStat();

	if (_debugLevel > 0)
		return 0;

	orgPrintf("Print Level [%s => ", strStat[print_lvl]);
	switch(print_lvl)
	{
		case PRINT_LVL_NON:
			print_lvl = PRINT_LVL_ALL; break;
		case PRINT_LVL_DBG:
			print_lvl = PRINT_LVL_NON; break;
		case PRINT_LVL_ALL:
			print_lvl = PRINT_LVL_DBG; break;
	}

	if (rc == -1)
		*pInKey = PC_KEY_F08;

	xlibc_setPrintStat(print_lvl);
	orgPrintf("%s]\n", strStat[print_lvl]);
	return rc;
}

static int _workF09(int* pInKey)
{
	int rc = 0;
	bool mode = cmd_getDebugMode();

	if (_debugLevel > 0)
		return 0;
	if      (mode == true)  mode = false;
	else if (mode == false) mode = true;

	cmd_setDebugMode(mode);

	if (rc == -1)
		*pInKey = PC_KEY_F09;

	rprint1n("^y^Debug mode %sd", _EnableMsgs[mode]);
	return rc;
}

static int _workF10(int* pInKey)
{
	int rc = 0;

	if(_debugEnable == false)
		return 0;

	if(_debugLevel == 0) cmdDebugMain();
	else                 {_debugLevel = 0; rprint1n("^y^ Exit debug menu");}

	if (rc == -1)
		*pInKey = PC_KEY_F10;

	return rc;
}

/*-----------------------------------------------------------------------------
 *	debug shell thread
------------------------------------------------------------------------------*/
void _debugShell(void)
{
	int		inKey;

	tprint0n("^y^Enter Debug Mode: if you want to exit from debug, input ");
	rprint1n("^r^'x'");
	xlibc_term_setmode(0, ICANON|ECHO);

	usleep(10);
	while(1)
	{
		inKey = xlibc_getc(stdin);

		if((_debugEnable == false)&&(inKey == 0x0a))
		{
			orgPrintf("Debug message has been suppressed\n");
		}
		else
		{
			dbgprint("0x%08x", inKey);
		}
	}
};

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void cmd_setDebugLevel(int val)
{
	_debugLevel = val;
}

int cmd_getDebugLevel(void)
{
	return _debugLevel;
}

bool cmd_setDebugMode(bool flag)
{
	static bool saved = false;

	saved = _debugEnable;

	if(flag == false)
		_debugEnable = false;
	else
		_debugEnable = true;

	return saved;
}

bool cmd_getDebugMode(void)
{
	return _debugEnable;
}

void cmd_setExcMode(bool mod)
{
	_excMode = mod;
}

void cmdHelpMenu(CMD_DBG_MENU_T *pMenu, int help_mode)
{
	_helpMenu(pMenu, help_mode);
}

void cmdDebugMain(void)
{
	static int prnt_stat;

	prnt_stat = xlibc_setPrintStat(PRINT_LVL_DBG);
	xlibc_term_setmode(ICANON|ECHO, ICANON|ECHO);
	_mainMenu(NULL, NULL);
	xlibc_term_setmode(0,           ICANON|ECHO);
	xlibc_setPrintStat(prnt_stat);
}

#include	<pthread.h>
void init_debug_system(void)
{
	pthread_attr_t	attr;
	pthread_t		tid;
	int				err;

	(void)xlibc_reg_cmdkey(PC_KEY_F08, (void*)_workF08);
	(void)xlibc_reg_cmdkey(PC_KEY_F09, (void*)_workF09);
	(void)xlibc_reg_cmdkey(PC_KEY_F10, (void*)_workF10);


	err = pthread_attr_init(&attr);
	if(0 != err)
		return;
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(0 == err)
	{
		pthread_create_ext("keyboard", &tid, &attr, (void*(*)(void*))_debugShell, NULL);
	}
	pthread_attr_destroy(&attr);

}

/*-----------------------------------------------------------------------------
 *	Debug Menu Define
------------------------------------------------------------------------------*/
static CMD_DBG_MENU_T	_SDM_SysInfo[] = {
//  Name,     Next,                          OPTS,  TYPE,     Description,                   Help
  { "none",	  (void *)NULL,                     0,  DMT_DESC, "systemInfo",                  NULL },
  { "task",	  (void *)debug_ShowThread,         0,  DMT_ARG1, "Show Thread status",
	"taskId|taskName|-1"							DMT_HELP_TAG
	": Use -1 to display all task information"		DMT_HELP_TAG
  },
  {  NULL,	  (void *)NULL,                     0,  DMT_TERM,  NULL,                         NULL }
};

static CMD_DBG_MENU_T _SDM_System[] = {
//  Name,     Next,                          OPTS,  TYPE,     Description,                   Help
  { "none",	  (void *)NULL,                     0,  DMT_DESC, "debugSystem",                 NULL },
  {  NULL,	  (void *)NULL,                     0,  DMT_TERM,  NULL,                         NULL }
};


static CMD_DBG_MENU_T	_SDM_Global[] = {
//  Name,     Next,                          OPTS,  TYPE,     Description,                   Help
  { "none",	  (void *)NULL,                     0,  DMT_DESC, "globalCmd",                   NULL },
  { "show",	  (void *)&_SDM_SysInfo,            0,  DMT_MENU, "Show Various status",         NULL },
  { "sh",	  (void *)debug_os_shell,           0,  DMT_ARGCV,"Excute OS shell",             NULL },
  { "md",	  (void *)debug_MemoryDump,         0,  DMT_ARGV, "Memory dump",
	"[address [size]]"								DMT_HELP_TAG
	"- Dump range is [addres, address+size)"		DMT_HELP_TAG
	"- Default dump size is 64"						DMT_HELP_TAG
  },
  { "dsm",	  (void *)debug_Disassemble,        0,  DMT_ARGV, "Disassemble memory",
    "address [count]"								DMT_HELP_TAG
	"- address : Hex/Dec address     => Use address   "					DMT_HELP_TAG
	"            SymbolName[+offset] => Use address of symbol"			DMT_HELP_TAG
	"            .                   => Use previous address in stack"	DMT_HELP_TAG
  },
  { "pt",	  (void *)debug_toggle_time,        0,  DMT_NONE, "Show time info of printf",    NULL },
  { "exp",	  (void *)debug_ExpTest,            0,  DMT_NONE, "Exp Test",                    NULL },
  {  NULL,	  (void *)NULL,                     0,  DMT_TERM,  NULL,                         NULL }
};

static CMD_DBG_MENU_T _SDM_Test[] = {
//  Name,     Next,                          OPTS,  TYPE,     Description,                   Help
  { "none",	  (void *)NULL,                     0,  DMT_DESC, "testMenu",                    NULL },
  { "color",  (void *)debug_TestColor,          0,  DMT_NONE, "Show strings with colors", 	 NULL },
  { "bt",     (void *)debug_Dummy,              0,  DMT_NONE, "Test Dummy Function", 	     NULL },
  { "crash",  (void *)debug_Crash,              0,  DMT_NONE, "Test System Crash", 	         NULL },
  {  NULL,	  (void *)NULL,                     0,  DMT_TERM,  NULL,                         NULL }
};

static CMD_DBG_MENU_T	_SDM_Main[] = {
//  Name,     Next,                          OPTS,  TYPE,     Description,                   Help
  { "none",	  (void *)NULL,                     0,  DMT_DESC, "debugMain",                   NULL },
  { "sys",	  (void *)&_SDM_System,             0,  DMT_MENU, "System debug",    			 NULL },
  { "test",   (void *)&_SDM_Test,               0,  DMT_MENU, "Test basic functions",        NULL },
  { "info",	  (void *)&_SDM_SysInfo,            0,  DMT_MENU, "Sample hard link menu",       NULL },

  { "bt",     (void *)&_SDM_Test,               0,  DMT_LINK, "Test Dummy Function", 	     "bt" },
  { "crash",  (void *)&_SDM_Test,               0,  DMT_LINK, "Test System Crash", 	      "crash" },
  { "task",   (void *)&_SDM_SysInfo,            0,  DMT_LINK, "Show Thread status",	       "task" },
  { "mmsw",   (void *)&_SDM_System,             0,  DMT_LINK, "Memory system switch",      "mmsw" },
  { "mmstat", (void *)&_SDM_System,             0,  DMT_LINK, "Display Memory OP List",  "mmstat" },
  { "mtest",  (void *)&_SDM_Test,               0,  DMT_LINK, "Memory Performance Test",  "mtest" },
  {  NULL,	  (void *)NULL,                     0,  DMT_TERM,  NULL,                         NULL }
};
