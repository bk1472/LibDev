/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** dsm.c
 *
 *	disassembler common code
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
static bool is_dsm_open = false;

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
int	dsmInst(inst_t* pInst, uint64_t address, void* printDSM)
{
	cs_err			err;
	static csh		handle;
	int				addrInc = 0;
	size_t			count;
	cs_insn*		insn;
	dsm_prnt_t		dsmPrint = NULL;
	platform_t*		pPlat    = getDsmPlatform();

	if(is_dsm_open == false)
	{
		err = cs_open(pPlat->arch, pPlat->mode, &handle);
		if(err)
			return 0;

		is_dsm_open = true;
	}

	if(printDSM == NULL) dsmPrint = (dsm_prnt_t)rprint0n;
	else                 dsmPrint = printDSM;

	if(pPlat->opt_type != CS_OPT_INVALID && pPlat->opt_value != CS_OPT_OFF)
		cs_option(handle, pPlat->opt_type, pPlat->opt_value);
	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

	count = cs_disasm(handle, (unsigned char*)pInst, 40, address, 0, &insn);
	if(count)
	{
		int n,k;
		char hex[50] = {0,};
		for(n = 0, k = 0; n < insn[0].size; n++, k +=2)
		{
			snprintf(&hex[k], 50-k, "%02x", insn[0].bytes[n]);
		}
		hex[k] = '\0';
		print_dsm_data(dsmPrint,  insn[0].address, &hex[0], insn[0].mnemonic, insn[0].op_str);
		addrInc = insn[0].size;
	}
	else
	{
		dsmPrint("%lx  %0lx\n", (uint64_t)pInst, *(uint64_t*)pInst);

		addrInc =sizeof(uint64_t);
	}

	cs_free(insn, count);

	//cs_close(&handle);

	return addrInc;
}
