/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_output.c
 *
 *	System output log control
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/
#define PRINT_DIGIT_TIME_MODE		2

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"cmd_shell.h"
#include	"thread.h"
#include	"xlibc.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define	DBG_BUFSZ			512
#define GHD_BUFSZ			160


#define	CRNL_MASK			0x01
#define	HEAD_MASK			0x02
#define	TIME_MASK			0x04

#define	DBG_CRNL			"\n"

#define	DBG_FMT_R0N			(                                0)
#define	DBG_FMT_R1N			(                        CRNL_MASK)
#define	DBG_FMT_H0N			(            HEAD_MASK            )
#define	DBG_FMT_H1N			(            HEAD_MASK | CRNL_MASK)
#define	DBG_FMT_T0N			(TIME_MASK                        )
#define	DBG_FMT_T1N			(TIME_MASK             | CRNL_MASK)
#define	DBG_FMT_TH0N		(TIME_MASK | HEAD_MASK            )
#define	DBG_FMT_TH1N		(TIME_MASK | HEAD_MASK | CRNL_MASK)

#define	DBG_FMT_DEF0N		DBG_FMT_TH0N
#define	DBG_FMT_DEF1N		DBG_FMT_TH1N


#if	  (PRINT_DIGIT_TIME_MODE == 1)
#  define LL_UDIV_MS		   1
#  define LL_UDIV_SEC		1000
#elif (PRINT_DIGIT_TIME_MODE == 2)
#  define LL_UDIV_MS		   1
#  define LL_UDIV_SEC		1000000
#else
#  define LL_UDIV_MS		  10
#  define LL_UDIV_SEC		 100
#endif /* PRINT_DIGIT_TIME_MODE */

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#ifndef isprint
#define	isprint(c)			( ((c) >= 32) && ((c) < 127) )
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static pthread_mutex_t	_PrntLockMtx      = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static int				_print_stat        = PRINT_LVL_ALL;

static char *dbg_format[10] = {
			"%s%s%s",								//	DBG_FMT_R0N
			"%s%s%s"					DBG_CRNL,	//	DBG_FMT_R1N
			"%-8.8s] %s%s%s",						//	DBG_FMT_H0N
			"%-8.8s] %s%s%s"			DBG_CRNL,	//	DBG_FMT_H1N
#if	  (PRINT_DIGIT_TIME_MODE == 1)
			"%03d.%03d] %s%s%s",					//	DBG_FMT_T0N
			"%03d.%03d] %s%s%s"		 	DBG_CRNL,	//	DBG_FMT_T1N
			"%03d.%03d:%-08s] %s%s%s",				//	DBG_FMT_TH0N
			"%03d.%03d:%-08s] %s%s%s"	DBG_CRNL,	//	DBG_FMT_TH1N
#elif (PRINT_DIGIT_TIME_MODE == 2)
			"%06d.%06d] %s%s%s",					//	DBG_FMT_T0N
			"%06d.%06d] %s%s%s"		 	DBG_CRNL,	//	DBG_FMT_T1N
			"%06d.%06d:%-08s] %s%s%s",				//	DBG_FMT_TH0N
			"%06d.%06d:%-08s] %s%s%s"	DBG_CRNL,	//	DBG_FMT_TH1N
#else
			"%02d.%02d] %s%s%s",					//	DBG_FMT_T0N
			"%02d.%02d] %s%s%s"		 	DBG_CRNL,	//	DBG_FMT_T1N
			"%02d.%02d:%-08s] %s%s%s",				//	DBG_FMT_TH0N
			"%02d.%02d:%-08s] %s%s%s"	DBG_CRNL,	//	DBG_FMT_TH1N
#endif /* PRINT_DIGIT_TIME_MODE */
			"%s%s%s\n",								//	DBG_FMT_R0N
			"%s%s%s\n"					DBG_CRNL	//	DBG_FMT_R1N
};

static char	*clrStrings[] = {   /* color    : index: Value      */
			"\x1b[30m",         /* blac[k]  :    0 : 30         */
			"\x1b[31m",         /* [r]ed    :    1 : 31         */
			"\x1b[32m",         /* [g]reen  :    2 : 32         */
			"\x1b[33m\x1b[40m", /* [y]ellow :    3 : 33         */
			"\x1b[34m",         /* [b]lue   :    4 : 34         */
			"\x1b[35m",         /* [p]urple :    5 : 35         */
			"\x1b[36m",         /* [c]yan   :    6 : 36         */
			"\x1b[37m",         /* gr[a]y   :    7 : 37==0 -> x */
			"\x1b[37m\x1b[40m", /* blac[K]  :    0 : 40         */
			"\x1b[37m\x1b[41m", /* [R]ed    :    1 : 41         */
			"\x1b[30m\x1b[42m", /* [G]reen  :    2 : 42         */
			"\x1b[30m\x1b[43m", /* [Y]ellow :    3 : 43         */
			"\x1b[37m\x1b[44m", /* [B]lue   :    4 : 44         */
			"\x1b[37m\x1b[45m", /* [P]urple :    5 : 45         */
			"\x1b[30m\x1b[46m", /* [C]yan   :    6 : 46         */
			"\x1b[37m\x1b[40m", /* gr[A]y   :    7 : 47==0 -> x */
			"\x1b[0m"           /* Reset  : Reset fg coloring */
			};

/*-----------------------------------------------------------------------------
 * Static Function Definition
------------------------------------------------------------------------------*/
static void __lockPrnt(void)
{
	(void) pthread_mutex_lock(&_PrntLockMtx);
	return;
}

static void __unlockPrnt(void)
{
	(void) pthread_mutex_unlock(&_PrntLockMtx);
	return;
}

static int __dbg_vsprint(FILE* fp, uint32_t head, const char *format , va_list args)
{
	char		dbgbuf[DBG_BUFSZ];
	uint32_t	type, sec, msec;
	uint64_t	tick;
	char*		color1   = (char*)"";
	char*		color2   = (char*)"";
	char		t_name[THREAD_NAME_SZ];
	int32_t		count    = 0;
	pid_t		tid;

	if(orgFprintf == NULL)
	{
		if((orgFprintf = xlibc_getOrigFunc("fprintf")) == NULL)
			return 0;
	}

	if(NULL == fp)
	{
		fp = stderr;
	}

	if (PRINT_LVL_NON == _print_stat)
		return 0;

	if(*format == '!')
		return 0;

	if ( (format[0]=='^') && (format[2]=='^') )
	{
		int	colorIndex = -1;
		switch (format[1])
		{
			case 'k': case 'K': colorIndex = 0; break;
			case 'r': case 'R': colorIndex = 1; break;
			case 'g': case 'G': colorIndex = 2; break;
			case 'y': case 'Y': colorIndex = 3; break;
			case 'b': case 'B': colorIndex = 4; break;
			case 'p': case 'P': colorIndex = 5; break;
			case 'c': case 'C': colorIndex = 6; break;
			case 'a': case 'A': colorIndex = 7; break;
		}

		if (colorIndex >= 0)
		{
			if (!(format[1] & 0x20)) colorIndex += 8;
			format += 3;
			color1  = clrStrings[colorIndex];
			color2  = clrStrings[16];
		}
	}

   	count = vsnprintf( &dbgbuf[0], DBG_BUFSZ-14, format, args );
	type  = (uint32_t)head;

	if (type > DBG_FMT_TH1N)
	{
		type = DBG_FMT_DEF1N;
	}

	if (type & TIME_MASK)
	{
		#if	(LL_UDIV_MS == 1) && (PRINT_DIGIT_TIME_MODE == 2)
		tick = xlibc_read_usticks();
		#else
		tick = xlibc_read_msticks();
		#endif

		#if	(LL_UDIV_MS == 1)
		#if (PRINT_DIGIT_TIME_MODE == 2)
		msec = (uint32_t)((tick%1000000L)               );
		sec  = (uint32_t)((tick/1000000L) % LL_UDIV_SEC );
		#else
		msec = (uint32_t)((tick%1000L)                  );
		sec  = (uint32_t)((tick/1000L) % LL_UDIV_SEC    );
		#endif
		#else
		msec = (uint32_t)((tick%1000L) / LL_UDIV_MS     );
		sec  = (uint32_t)((tick/1000L) % LL_UDIV_SEC    );
		#endif
	}
	else
	{
		tick = 0;
		msec = 0;
		sec  = 0;
	}

	__lockPrnt();

	if (type & HEAD_MASK)
	{
		/**
		 * BK1472 Comment:
		 * 나중에 thread manager 를 만들때 thread 별 info. 관리자로 부터
		 * 정보를 얻어오는 방식으로 수정이 필요함
		 * 2018.03.23
		 */
		char* nm = get_threadName(get_current());
		if (nm == NULL)
		{
			tid = get_tid(get_current());
			if(tid < 0) snprintf(&t_name[0], THREAD_NAME_SZ-1, "t.noname");
			else        snprintf(&t_name[0], THREAD_NAME_SZ-1, "t.%06x", tid);
		}
		else
			snprintf(&t_name[0], THREAD_NAME_SZ-1, "%s", nm);
		t_name[THREAD_NAME_SZ-1] = '\0';
	}
	switch(type)
	{
		case DBG_FMT_R0N:
		case DBG_FMT_R1N:
			(*orgFprintf)(fp, dbg_format[type], color1, dbgbuf, color2);
			break;

		case DBG_FMT_H0N:
		case DBG_FMT_H1N:
			(*orgFprintf)(fp, dbg_format[type], t_name, color1, dbgbuf, color2);
			break;

		case DBG_FMT_T0N:
		case DBG_FMT_T1N:
			(*orgFprintf)(fp, dbg_format[type], sec, msec, color1, dbgbuf, color2);
			break;

		case DBG_FMT_TH0N:
		case DBG_FMT_TH1N:
			(*orgFprintf)(fp, dbg_format[type], sec, msec, t_name, color1, dbgbuf, color2);
			break;
	}
	__unlockPrnt();

	return (count);
}

static inline void init_crc32(uint32_t *crc_tbl)
{
	#define CRC32_POLY 0x04c11db7     /* AUTODIN II, Ethernet, & FDDI */
	int			i, j;
	uint32_t	c;

	for (i = 0; i < 256; ++i)
	{
		for (c = i << 24, j = 8; j > 0; --j)
			c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
			crc_tbl[i] = c;
	}
}

static uint32_t		crc32_table[256] = {0,};
uint32_t calccrc32(uint8_t *buf, int len)
{
	uint8_t				*p;
	uint32_t			crc;

	if (!crc32_table[1])						/* if not already done, */
		init_crc32(&crc32_table[0]);			/* build table			*/

	crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */
	for (p = buf; len > 0; ++p, --len)
		crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];
	return(crc);
	// return ~crc;            /* transmit complement, per CRC-32 spec */
}

void _genHexDump(FILE* fp, const char *name, void *vcp, int size, fprintf_proc_t pFunc)
{
	#if    (BIT_WIDTH == BIT_64)
	 #define HPOS_INIT		26
	 #define CPOS_OFFS		64
	 #define PRNTMSG		"\t0x%016lx(%04x):"
	#elif  (BIT_WIDTH == BIT_32)
	 #define HPOS_INIT		18
	 #define CPOS_OFFS		56
	 #define PRNTMSG		"\t0x%08x(%04x):"
	#endif
	static int		_match_byteOrder = 1;
	static uint8_t	n2h[] = "0123456789abcdef";
	int				i, hpos, cpos;
	int				id[4];
	char			buf[GHD_BUFSZ+1] = {0};
	uint8_t			*cp, uc;
	uint32_t		crc_val = 0;

	if (cmd_getDebugMode() == false)
		return;

	if(NULL == fp)fp = stderr;
	cp   = (uint8_t *) vcp;
	crc_val = calccrc32(cp, size);

	snprintf(buf, GHD_BUFSZ, "%s(Size=0x%x, CRC32=0x%08x)", name, size, crc_val);
	buf[GHD_BUFSZ] = 0;
	(*pFunc)(fp, "%s\n",buf);

	if (size == 0) return;

	memset(buf, ' ', GHD_BUFSZ);

	cp = (uint8_t*)vcp;
	hpos = cpos = 0;
	for (i=0; i < size; ) {

		if ((i % 16) == 0) {
			snprintf(buf, GHD_BUFSZ, PRNTMSG, (long)vcp+i, i);
			hpos = HPOS_INIT;
		}

		if ((i % 4)  == 0) buf[hpos++] = ' ';

		#if (ENDIAN_TYPE == LITTLE_ENDIAN)
		if (_match_byteOrder)
		{
			id[0] = 3; id[1] = 2; id[2] = 1; id[3] = 0;
		}
		else
		#endif
		{
			id[0] = 0; id[1] = 1; id[2] = 2; id[3] = 3;
		}

		if ((i+0)<size) {uc=cp[i+id[0]]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+1)<size) {uc=cp[i+id[1]]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+2)<size) {uc=cp[i+id[2]]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+3)<size) {uc=cp[i+id[3]]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}

		cpos = (i%16) + CPOS_OFFS;

		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}

		if ((i%16) == 0) {
			buf[cpos] = 0x00;
			(*pFunc)(fp, "%s\n",buf);
		}
	}
	buf[cpos] = 0x00;
	if ((i%16) != 0) {
		for ( ; hpos < CPOS_OFFS; hpos++)
			buf[hpos] = ' ';
		(*pFunc)(fp, "%s\n",buf);
	}
}

/*-----------------------------------------------------------------------------
 * API Function Definition
------------------------------------------------------------------------------*/
int xlibc_setPrintStat(int new)
{
	int old = _print_stat;

	if (new > PRINT_LVL_ALL)
		new = PRINT_LVL_ALL;
	if (new < PRINT_LVL_NON)
		new = PRINT_LVL_NON;

	xlibc_setCoutStat(new);
	_print_stat = new;

	return old;
}

int xlibc_getPrintStat(void)
{
	return _print_stat;
}

int dbgprint(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stdout, (uint32_t)DBG_FMT_DEF1N, format, ap);
    va_end(ap);
    return count;
}

int tprint0n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stdout, (uint32_t)DBG_FMT_DEF0N, format, ap);
    va_end(ap);
    return count;
}

int tprint1n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stdout, (uint32_t)DBG_FMT_DEF1N, format, ap);
    va_end(ap);
    return count;
}

int	rprint0n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stdout, (uint32_t)DBG_FMT_R0N, format, ap);
    va_end(ap);
    return count;
}

int	rprint1n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stdout, (uint32_t)DBG_FMT_R1N, format, ap);
    va_end(ap);
    return count;
}

int eprint0n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stderr, (uint32_t)DBG_FMT_R0N, format, ap);
    va_end(ap);
    return count;
}

int eprint1n(const char *format , ... )
{
    int		count;
    va_list	ap;

    va_start( ap, format);
    count = __dbg_vsprint(stderr, (uint32_t)DBG_FMT_R1N, format, ap);
    va_end(ap);
    return count;
}

int ext_vprintf(const char* format, va_list ap)
{
    return  __dbg_vsprint(stdout, (uint32_t)DBG_FMT_DEF0N, format, ap);
}

void hexdump(const char* name, void* vcp, int size)
{
	_genHexDump(stdout, name, vcp, size, orgFprintf);
}

void hexdump_f(FILE* fp, const char* name, void* vcp, int size)
{
	_genHexDump(fp, name, vcp, size, orgFprintf);
}
