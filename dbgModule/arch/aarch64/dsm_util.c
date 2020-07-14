/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** dsm_util.c
 *
 *	disassembler for aarch64 platform
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
#include	"dsm.h"

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
static platform_t platform =
{
	.arch      = CS_ARCH_ARM64,
	.mode      = CS_MODE_ARM,
	.opt_type  = CS_OPT_INVALID,
	.opt_value = CS_OPT_OFF,
};

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
platform_t* getDsmPlatform(void)
{
	return &platform;
}

void print_dsm_data(dsm_prnt_t dsmPrint, uint64_t addr, char* hexcode, char* opcode, char* operand)
{
	if(dsmPrint)
		dsmPrint("%0lx  %-18s  %s\t%s\n", addr, hexcode, opcode, operand);
}
