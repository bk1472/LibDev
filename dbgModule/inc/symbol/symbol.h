/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** symbol.h
 *
 *	symbol api libearies
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 29
 *	@note
 *	@see
 */
#ifndef __SYMBOL_H__
#define __SYMBOL_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdint.h>
#include	"ext_common.h"
#include	"xlibc_output.h"
#include	"dwarf.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define USE_DYM_SYMBOL		0

/*-----------------------------------------------------------------------------
 *	Macro Definitions
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
/*addr2line.c*/
extern int		searchLineInfo		(char **ppDebugLine, uint64_t *pSize, uint64_t srchAddr, char **ppFileName);
extern int		addr2line			(uint64_t addr, char **ppFileName);

/*symbol.c*/
extern void		init_symbol			(void);
extern bool		is_symbol_load		(void);
extern char*	getLibName			(void);
extern uint64_t	findSymByAddr		(uint64_t addr, char **pSymName);
extern uint64_t	findDmgSymByAddr	(uint64_t addr, char **pSymName, char* dmgBuf, int size);
extern char*	findSymbolName		(uint64_t addr);
extern uint64_t	findSymByName		(const char *symName);

/*syn_sym.c*/
extern uint64_t	findDynSymByAddr	(uint64_t addr, char **pSymName, char **pLibName);
extern uint64_t	findDynSymByName	(const char *symName, char **pLibName);
extern char*	findDynSymbolName	(uint64_t addr);

#ifdef __cplusplus
}
#endif

#endif/*__SYMBOL_H__*/
