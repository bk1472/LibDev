/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file cmd_input.c
 *
 *	Command input APIs compilation
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
#include	"xlibc.h"
#include	"cmd_shell.h"

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

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
uint32_t	cmd_getIntegerInput(const char* comment, int32_t* pVal)
{
	char	inputStr[20];

	if(pVal == NULL)
		return -1;

	rprint0n("%s: 0x", comment);
	(void)xlibc_gets(inputStr, 19, stdin);

	*pVal = strtol(inputStr, (char**)NULL, 10);

	return 0;
}

uint32_t	cmd_getHexInput(const char* comment, uint32_t* pVal)
{
	char	inputStr[20]="0x";

	if(pVal == NULL)
		return -1;

	rprint0n("%s: 0x", comment);
	(void)xlibc_gets(&inputStr[2], 17, stdin);

	*pVal = strtoul(inputStr, (char**)NULL, 16);

	return 0;
}

uint32_t	cmd_getDecimalInput(const char* comment, uint32_t* pVal)
{
	char	inputStr[20];

	if(pVal == NULL)
		return -1;

	rprint0n("%s: 0x", comment);
	(void)xlibc_gets(inputStr, 19, stdin);

	*pVal = strtoul(inputStr, (char**)NULL, 10);

	return 0;
}

uint32_t	cmd_getDoubleInput(const char* comment, double* pVal)
{
	char	inputStr[20];

	if(pVal == NULL)
		return -1;

	rprint0n("%s: 0x", comment);
	(void)xlibc_gets(inputStr, 19, stdin);

	*pVal = strtod(inputStr, (char**)NULL);

	return 0;
}

uint32_t	cmd_getFloatInput(const char* comment, float* pVal)
{
	char	inputStr[20];

	if(pVal == NULL)
		return -1;

	rprint0n("%s: 0x", comment);
	(void)xlibc_gets(inputStr, 19, stdin);

	*pVal = strtof(inputStr, (char**)NULL);

	return 0;
}

uint32_t	cmd_readCmdString(const char *prompt, char *cmdStr, size_t cmdLen)
{
	size_t	inpLen;

	rprint0n("%s", prompt);
	(void)xlibc_gets(&cmdStr[0], cmdLen-1, stdin);
	inpLen = strlen(cmdStr);
	if ((inpLen > 0) && (cmdStr[inpLen-1] == '\n'))
	{
		inpLen--;
		cmdStr[inpLen] = '\x0';
	}
	return(inpLen);

}

uint32_t	cmd_readNumber(const char *prompt, int digits, uint32_t val)
{
	                    /* 0  1  2  3  4  5  6  7  8 */
	static char	dtag[] = { 1, 2, 3, 4, 5, 7, 8, 9, 9};
	char	inputStr[20];
	char	*cp = &inputStr[0];

	rprint0n("%s: 0x%0*x(%*d) ==> ", prompt, digits, val, dtag[digits], val);

	(void)xlibc_gets(cp, 19, stdin);
	cp = (char *)strtrim(cp);

	if ((*cp != '\r') && (*cp != '\n'))
		val = (uint32_t) strtoul(cp, (char **) NULL, 0);

	return(val);
}
