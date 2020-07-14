/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file root.c
 *
 *  dbgModule main start root file
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
#include	"xlibc_hooklib.h"
#include	"xlibc_output.h"
#include	"cmd_shell.h"
#include	"threadlib.h"
#include	"symbol.h"
#include	"ipc_comm.h"

#include	<sys/stat.h>
#include	<fcntl.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define INIT_SYS_SZ		(sizeof(_initSys)/sizeof(_initSys[0]))

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef void (*init_func_t)(void);

typedef struct
{
	char*		comment;
	init_func_t	pfunc;
} init_sys_t;

/*-----------------------------------------------------------------------------
 *	External Function prototypes declarations
------------------------------------------------------------------------------*/
extern void				init_except			(void);

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static void				_drv_init			(void);

static int				_lgdrv_fd			= -1;
static init_sys_t		_initSys[] =
{
	{ "LG DRvier",   _drv_init              },
	{ "Hook System", xlibc_init_hook_system },
	{ "Symbol",      init_symbol            },
	{ "Thread",      init_thread_system     },
	{ "Debug",       init_debug_system      },
	{ "EXCEPT",      init_except            },
	{ "IPC",         init_comm              },
};

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static void __CONT__(101) _start_dbgModule(void)
{
	int		n;

	for(n = 0; n < INIT_SYS_SZ; n++)
	{
		_initSys[n].pfunc();
	}
	regist_thread_info("main", (void*)_start_dbgModule, NULL, 0x800000);
}



static void _drv_init(void)
{
	if((_lgdrv_fd = open("/dev/lgdriver", O_RDWR, 0777)) < 0)
		_lgdrv_fd = -1;

	return;
}

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
int get_drv_fd(void)
{
	return _lgdrv_fd;
}

void null_function() {}
