/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/
/** demangle.h
 *
 *	demangling C++ symbol
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 29
 *	@note
 *	@see
 */
#ifndef __DEMANGLE_H__
#define __DEMANGLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ext_common.h"

extern char *demangle_symbol (const char* mangled, char *dmgBuf, int size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif/*__DEMANGLE_H__*/
