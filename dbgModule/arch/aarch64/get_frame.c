/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** get_frame.c
 *
 *	get stack frame for aarch64
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 4
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/
#define BT_DBG_LVL			0

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc.h"
#include	"thread.h"
#include	"symbol.h"
#include	"backtrace.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define AARCH64_PUSH_SP			(0xFD7B00A9)		// stp x29, x30 [sp , #xx]
#define AARCH64_SUB_SP			(0xFF0300D1)		// sub sp, sp, #xx
#define AARCH64_RET				(0xC0035FD6)

#define AARCH64_PUSH_MSK		(0xFFFF00FF)
#define AARCH64_SUB_SP_MSK		(0xFF0300FF)		// sub sp, sp, #xx
#define AARCH64_PUSH_IMM_MSK	(0x0000FF00)
#define AARCH64_SUB_IMM_MSK		(0x00FCFF00)

#define BTDBGPRNT(_lvl, _fmt, _arg...)		do{ if(_verboseBt >= (_lvl)) rprint1n(_fmt, ##_arg); }while(0)
#define BTDBGHEXDUMP(_lvl, _ptr, _size)		do{ if (_verboseBt >= (_lvl)) hexdump("Stack", (void*)((_ptr)), (_size)); } while(0)

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static int	_verboseBt   = BT_DBG_LVL;

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static uint32_t _get32bit_op(uint8_t* pc)
{
	/* instruction should be swapped */
	uint8_t		op[4];
	uint32_t	opcode;

	op[0] = *(pc+3);op[1] = *(pc+2);
	op[2] = *(pc+1);op[3] = *(pc+0);
	opcode = *((uint32_t*)&op[0]);

	return opcode;
}
/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/
uint64_t* aarch64_get_pc(void)
{
	return  (uint64_t*)__builtin_return_address(0);
}

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
int32_t get_frame_size(uint64_t* sp, uint64_t* usl, uint64_t* pc, uint64_t** ra)
{
	uint32_t*	_32_pc  = (uint32_t*)pc;
	uint64_t*	tmpSp;
	int			iterLmt = 1000;
	int32_t		frame   = 0;
	bool		sub_mod = false;



	if(false == isValidAddr((uint64_t)sp))
		return 0;

	BTDBGPRNT(1, "^Y^[1] Back Loop for find function prologue");
	do
	{
		uint32_t	opcode;

		if (false == isValidPc((uint64_t)_32_pc))
			return 0;

		opcode = _get32bit_op((uint8_t*)_32_pc);

		/**
		 * stack frame을 발견하지 못하고 이전 함수의 return 문 또는 nop를 만났을 때
		 */
		if(opcode == AARCH64_RET)
		{
			break;
		}

		if((opcode & AARCH64_PUSH_MSK) == AARCH64_PUSH_SP)
		{
			int32_t offs = ((opcode & AARCH64_PUSH_IMM_MSK) >> 8);

			if(offs & 0x80) //음수
			{
				offs = (0x000000FF & ~offs)+1;//보수를 취함
				if(offs & 0x40)
				{
					offs = (offs - 0x40) << 4;
					frame = offs;
				}
			}

			BTDBGPRNT(1, "^p^ op:%x frame:%d(%x)", opcode, frame, frame);
		}

		if ((opcode & AARCH64_SUB_SP_MSK) == AARCH64_SUB_SP)
		{
			int32_t value = opcode & AARCH64_SUB_IMM_MSK;
			int32_t val1  = ((value >> 22) << 4);
			int32_t val2  = ((value & 0xFF00) >> 2);
			int32_t	val3  = ((value & 0x40000) >> 6);

			frame = (val1 | val2 | val3);
			sub_mod = true;
			BTDBGPRNT(1, "^c^ op:%x frame:%d(%x)", opcode, frame, frame);
		}

		if(frame)
		{
			frame = (frame/sizeof(uint64_t));
			break;
		}
		_32_pc--;
	} while(iterLmt--);

	BTDBGPRNT(1, "^Y^[2] Find Return Address from Stack of Framed area: %x", frame*sizeof(uint64_t));

	BTDBGHEXDUMP(1, (sp-frame), 0x60*sizeof(uint64_t));
	if(sub_mod == true) tmpSp = &sp[-2];
	else                tmpSp = &sp[0];

	if(false == (isValidAddr((uint64_t)tmpSp)))
		return 0;

	if(   (true == (isValidPc  ((uint64_t)tmpSp[1])))
	  &&  (true == (isValidAddr((uint64_t)tmpSp[0]))))
	{
		*ra = (uint64_t*)tmpSp[1];;
	}
	else
	{
		return 0;
	}
	BTDBGPRNT(1, "%lx..%lx", tmpSp[0], tmpSp[1]);

	return (frame*sizeof(uint64_t));
}
