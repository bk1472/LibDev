/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** dsm.h
 *
 *	disassembler header file
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 19
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
#include	"xlibc_output.h"
#include	"platform.h"
#include	"capstone.h"

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	cs_arch			arch;
	cs_mode			mode;
	uint8_t*		code;
	size_t			size;
	cs_opt_type 	opt_type;
	cs_opt_value	opt_value;
} platform_t;

typedef uint64_t		inst_t;
typedef	int	(*dsm_prnt_t)(const char* fmt, ...);

#ifndef	__DSM_HEADER__
#define	__DSM_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

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
extern platform_t*		getDsmPlatform			(void);
extern void				print_dsm_data			(dsm_prnt_t dsmPrint, uint64_t addr, char* hexcode, char* opcode, char* operand);
extern int				dsmInst					(inst_t* pInst, uint64_t address, void* printDSM);

#ifdef __cplusplus
}
#endif

#endif/*__DSM_HEADER__*/
