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
 *	get stack frame for x64
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
/**
 *	X64에서 stack sub / add 명령어는 다음과 같다.
 *
 *	sub $im, %rsp :	48			83			EC
 *					0100|1000   1000|0011	1110|1100	xx			: im( 8 bit signed integer)
 *					48			81			EC
 *					0100|1000   1000|0001	1110|0100	xx xx xx xx	: im( 32 bit signed integer)
 *
 *	add $im, %rsp :	48			83			C4
 *					0100|1000   1000|0011	1100|1100	xx			: im( 8 bit signed integer)
 *					48			81			C4
 *					0100|1000   1000|0001	1100|0100	xx xx xx xx	: im( 32 bit signed integer)
 *
 *	stack 에서 frame size를 구한 후 return address를 구하기 위해서는
 *	현재 sp(stack pointer) 및 pc (program counter)를 가지고 pc를 code flow 의 역방향( CISC 이기 때문에 1byte 씩)
 *	으로 back 하면서 다음과 같은 순서로 찾는다.
 *
 *	(1) 0x488XEC (sub) / 0x488XC4 (add) pattern을 찾는다.	(X는 3 또는 1)
 *
 *	(2)	X 가 1이면 4 bytes 모드 , 3이면 1 byte 모드 im(숫자: signed)
 *		4bytes 모드는 Little endian 에 맞추어 byte swap을 해야 함.
 *
 *	(3)	X (1또는 3) 이후 1byte 가 add/sub를 결정 짓는데 EC(sub)이면 양의 정수,
 *		C4(add)이면 음의정수 이어야 함 : 1 byte 값 0x80 이면 128 을 의미 하며 sigend 값이므로 -0x80을 의미함
 *		즉 음수값을 add 함 (명령어를 줄이기 위한 방법)
 *
 *	(4)	frame size의 계산이 완료 되면 return address 는 frame pointer 바로 다음 주소에 저장 되어 있음 : (uint64_t)ra[1]
 *
 *                                                                       [    stack    ]
 *                                                                HIGH |------------------|
 *                                                                     +       ~~~        +
 *                                                                     +       ~~~        +
 *                                                                	sp |------------------|
 *                                                                     +                  +
 *                                                                     +                  +
 *                                                                     +       ~~~        +
 *                                                                     +       ~~~        +
 *                                                                     +                  +
 *                                                                     +       ~~~        +
 *                                                                     +                  +
 *                                                                	fp |------------------|
 *                                                                     +    return addr   +
 *                                                                     |------------------|   fp+1
 *                                                                     +       ~~~        +
 *                                                                     +       ~~~        +
 *                                                                 LOW |------------------|
 *
 *
 */

#define X64_ADRMOD_IMSP			(0x48800000)
#define X64_DS_SMALL			(0x00030000)
#define X64_DS_BIG				(0x00010000)
#define X64_OP_SUB				(0x0000EC00)
#define X64_OP_ADD				(0x0000C400)
#define X64_OP_RBP				(0x554889E5)				/* push %rbp (0x55):  movq %rsp, %rbp */
#define X64_ADRMOD_MASK			(0xFFFF0000)
#define X64_OPCODE_MASK			(0xFFFFFF00)
#define X64_ADD_SUB_MASK		(0x0000FF00)

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
uint64_t* x64_get_pc(void)
{
	return  (uint64_t*)__builtin_return_address(0);
}

uint64_t* x64_get_ra(void)
{
	return  (uint64_t*)__builtin_frame_address(0);
}

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
int32_t get_frame_size(uint64_t* sp, uint64_t* usl, uint64_t* pc, uint64_t** ra)
{
	uint8_t*	_8_pc     = (uint8_t*)pc;
	uint64_t*	tmpSp;
	int			iterLmt   = 10000;
	int			iterCnt   = 0;
	int32_t		frame     = 0;
	bool		_fndop_sp = false;

	if(false == isValidAddr((uint64_t)sp))
		return 0;

	BTDBGPRNT(1, "^Y^[1] Back Loop for find function prologue");
	do
	{
		uint32_t	opcode;
		int			bitSz  = 0;
		int			cmd    = 0;
		int			sign   = 1;

		if (false == isValidPc((uint64_t)_8_pc))
			return 0;

		opcode = _get32bit_op(_8_pc);

		BTDBGPRNT(2, "^p^ [%05d:%p]op: %08x mask: %08x, val1: %08x, val2: %08x",
										iterLmt,
										_8_pc,
										opcode,
										(opcode & X64_OPCODE_MASK),
										(X64_ADRMOD_IMSP|X64_DS_SMALL),
										(X64_ADRMOD_IMSP|X64_DS_BIG));

		if(_fndop_sp == true)
		{
			if      (opcode == X64_OP_RBP)
			{
				frame++;
				break;
			}
			else
			{
				uint16_t _16op = opcode >> 16;

				if((_16op & 0xFFF8) == 0x4150) // push register
				{
					frame++;
				}
			}
		}
		else
		{
			if      ((opcode & X64_ADRMOD_MASK) == (X64_ADRMOD_IMSP|X64_DS_SMALL))
				bitSz = X64_DS_SMALL;
			else if ((opcode & X64_ADRMOD_MASK) == (X64_ADRMOD_IMSP|X64_DS_BIG))
				bitSz = X64_DS_BIG;

			if      ((opcode & X64_ADD_SUB_MASK) == X64_OP_SUB)
				cmd = X64_OP_SUB;
			else if ((opcode & X64_ADD_SUB_MASK) == X64_OP_ADD)
				cmd = X64_OP_ADD;

			cmd = (bitSz | cmd);

			if(cmd)
			{
				switch(cmd)
				{
					case (X64_OP_SUB|X64_DS_SMALL):
						frame = (int32_t)(opcode&0x000000FF);
						BTDBGPRNT(1, "^c^pc(%p) SUB stack small mode: +%x", _8_pc, frame);
						break;
					case (X64_OP_SUB|X64_DS_BIG  ):
						frame = opcode & 0xFF;
						frame = (_8_pc[6] << 24) | (_8_pc[5] << 16) | (_8_pc[4] << 8) | frame;
						BTDBGPRNT(1, "^c^pc(%p) SUB stack big   mode: +%x", _8_pc, frame);
						break;
					case (X64_OP_ADD|X64_DS_SMALL):
						frame = (int32_t)(opcode&0x000000FF);
						if ((uint32_t)frame != 0x80) /*add 가 음수가 아닌 양수라는 것은 stack frame을 복원하는 행위다*/
						{
							frame = 0;
							break;
						}
						sign *=-1;
						BTDBGPRNT(1, "^c^pc(%p) ADD stack small mode: %c%x", _8_pc, (sign<0)?'-':'+', frame);
						break;
					case (X64_OP_ADD|X64_DS_BIG  ):
						BTDBGPRNT(1, "^r^pc(%p) ADD stack big   mode: error", _8_pc);
						frame = 0;
						return frame;
				}
			}
			if(0 != frame)
			{
				frame /= sizeof(uint64_t);
				_fndop_sp = true;
				/* compile option이 -O0 (optimize가 안된 case)이면
				 * stack 조작 명령어를 찾으면 되지만 optimize 가 된 경우는 찾을 수 없다.
				 */
				break;
			}
		}
		_8_pc--;
	} while (iterLmt--);

	BTDBGPRNT(1, "^Y^[2] Find Return Address from Stack of Framed area: %x", frame*sizeof(uint64_t));

	/* x64 return address = sp[frame + 1] */
	frame++;
	tmpSp = &sp[frame];

	for(iterCnt = 0; tmpSp < usl; tmpSp++, iterCnt++)
	{
		if(false == (isValidAddr((uint64_t)tmpSp)))
		{
			return 0;
		}
		*ra = (uint64_t*)tmpSp[0];
		BTDBGPRNT(2, "^g^[%05d] ra:%p pc:%p sp:%p", iterCnt, *ra, pc, sp);
		if(isValidPc((uint64_t)*ra) == true && pc != *ra)
			break;
	}
	if(tmpSp < usl)
		frame += iterCnt;
	else
		frame = 0;

	BTDBGPRNT(1, "^p^ra:%p @ sp:%p, sp[frame]:%p, frame(0x%x..%d)", *ra, sp, &sp[frame], frame, frame);
	BTDBGHEXDUMP(1, (sp), 0x60*sizeof(uint64_t));

	return (frame*sizeof(uint64_t));
}
