/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_util.c
 *
 *	System utility Library
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


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc.h"

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
 * Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Static Function Definition
------------------------------------------------------------------------------*/
inline static uint64_t _xlibc_clkread_nanosec (void)
{
	struct timespec curTime;

	(void)clock_gettime (CLOCK_MONOTONIC, &curTime);

	return ((uint64_t)curTime.tv_sec * 1000000000 + curTime.tv_nsec);
}

/*-----------------------------------------------------------------------------
 * API Function Definition
------------------------------------------------------------------------------*/
struct timespec xlibc_ms2abstime(int ms)
{
	struct timespec curTime;

	clock_gettime(CLOCK_REALTIME, &curTime);

	curTime.tv_sec  += (ms / 1000);
	curTime.tv_nsec += (ms % 1000) * 1000000;

	if (curTime.tv_nsec >= 1000000000)
	{
		curTime.tv_sec  += 1;
		curTime.tv_nsec -= 1000000000;
	}
	return (curTime);
}

uint64_t xlibc_read_usticks(void)
{
	return (_xlibc_clkread_nanosec()/(uint64_t)1000);
}

uint64_t xlibc_read_msticks(void)
{
	return (_xlibc_clkread_nanosec()/(uint64_t)1000000);
}

#define	MS2US(__ms)				((__ms) * 1000)

void xlibc_delay_us(unsigned int us)
{
	uint64_t	endUs;

	endUs = xlibc_read_usticks() + us;

	while (endUs > xlibc_read_usticks())
		;

	return;
}

void xlibc_delay_ms(unsigned int ms)
{
	unsigned int us;

	if (ms >= 1000)
	{
		sleep(ms/1000);
		us = (ms%1000)*1000;
		usleep(us);
	}
	else if (ms >= 1)
	{
		us = ms*1000;
		usleep(us);
	}
	else
		xlibc_delay_us(MS2US(ms));

	return;
}

unsigned long xlibc_suspend(unsigned int ms)
{
	#include <sched.h>
	if (ms == NO_WAIT)
	{
		sched_yield();
		/* Nothing to do, just ignore it	*/
	}
	else
	{
		struct timeval	timeout;

		timeout.tv_sec	=  ms / 1000;
		timeout.tv_usec	= (ms % 1000) * 1000;

		do
		{
			select(0, NULL, NULL, NULL, &timeout);
		} while (ms == WAIT_FOREVER);

	}
	return 0;
}

unsigned long strtoul2(const char *s, char **ptr, int base)
{
	unsigned long res;
	char *t;

	if (s == NULL)
	{
		if (ptr != NULL) *ptr = NULL;
		return 0;
	}

	/*	Remove heading spaces	*/
	while ((*s == '\t') || (*s == ' '))
	{
		if (*s == '\0') break;
		s++;
	}

	if ((base == 0) && (*s == 'b'))
	{
		base = 2;
		s++;
	}

	if (base == 2)
	{
		for (t=(char *)s; (*t == '0') || (*t == '1'); t++)
			;
	}
	else
	{
		for (t=(char *)s; isxdigit(*t) || (*t == 'x') || (*t == 'X'); t++)
			;
	}

	res = strtoull(s, ptr, base);

    switch (tolower(*t))
	{
		case 'r' : res = (res << 13); break;
		case 'k' : res = (res << 10); break;
		case 'm' : res = (res << 20); break;
		case 'g' : res = (res << 30); break;
	}

	return(res);
}

int str2argv(char *line, int limit, char **argv)
{
	register int	argc = 0;

	if (limit < 0) limit = 0;

    while (*line)
	{
		/* Skip Heading Spaces characters	*/
        while (*line && isspace(*line))
            line++;

		/* Check end of string				*/
        if (*line == '\0') break;

		/* Check end of argv list storage 	*/
		if ( (argc+1) >= limit) break;

		argc++;
        *argv++ = line;

		/* Find Next Space characters		*/
        while ( *line && !isspace(*line))
            line++;

		/* Check end of string				*/
        if (*line == '\0') break;

		/* Null Terminate argument			*/
        *line++ = '\0';
    }

	if (argc < limit)
		*argv = NULL;

	return(argc);
}

int str2indexInOpts(char **argv, const char *opts, int *output)
{
	char	*incp, *cp1, *cp2;
	int		res, len, min, max;
	bool	opt_flag = false;
	int		i;

	#define	END_DELIM	"^] \t"
	#define	OPT_DELIM	",|"
	#define	ALL_DELIM	OPT_DELIM	END_DELIM


	if ((opts == NULL) || (output == NULL))
		return -1;		/* Indicate Error */

	cp1 = (char *)opts;
	cp2 = strchr(cp1, '^');

	if (cp2) len = cp2 - cp1;
	else     len = strlen(cp1);

	*output  = -1;		/* Initialize output index as not found	*/

	if (sscanf(cp1, "[%d..%d]", &min, &max) == 2)
	{
		opt_flag = true;
		if (argv != NULL)
		{
			*output = (int) strtoul2(*argv, NULL, 0);
			if ((*output < min) || (*output > max))
			{
				*output = -1;
				return -1;
			}
		}
	}
	else if (sscanf(cp1, "%d..%d", &min, &max) == 2)
	{
		if (argv == NULL)
			return -1;

		*output = (int) strtoul2(*argv, NULL, 0);
		if ((*output < min) || (*output > max))
		{
			*output = -1;
			return -1;
		}
	}
	else if (sscanf(cp1, "[0x%x..0x%x]", &min, &max) == 2)
	{
		opt_flag = true;
		if (argv != NULL)
		{
			*output = (int) strtoul2(*argv, NULL, 16);
			if ((*output < (unsigned)min) || (*output > (unsigned)max))
			{
				*output = -1;
				return -1;
			}
		}
	}
	else if (sscanf(cp1, "0x%x..0x%x", &min, &max) == 2)
	{
		if (argv == NULL)
			return -1;

		*output = (int) strtoul2(*argv, NULL, 16);
		if ((*output < (unsigned)min) || (*output > (unsigned)max))
		{
			*output = -1;
			return -1;
		}
	}
	else
	{
		if ((cp1[0] == '[') && (cp1[len-1] == ']'))
		{
			opt_flag = true;
			cp1 = cp1 + 1;
			len = len - 2;
		}

		if ( (argv == NULL) || (*argv == NULL) )
		{
			/* Cancel search operation */
			*output = -1;
			if (!opt_flag) return -1;
			else           return  0;
		}

		res  = 0;
		incp = *argv;

		while (len > 0)
		{
			int		tRes = 0;
			void	*fmt = (void *)(((cp1[0]=='0') && (cp1[1]=='x')) ? (char *)"%x" : (char *)"%d");

			if (cp1[0] != '_')
				if (sscanf(cp1, (const char *)fmt, &tRes) == 1) res = tRes;

			for (i=0; (cp1 && incp[i]); i++)
			{
				if (cp1[i] != incp[i])
					break;
			}

			if ( !incp[i] && (!cp1[i] || strchr(ALL_DELIM, cp1[i])))
			{
				/* Option Mached */
				break;
			}

			for (cp2=cp1; cp2[0] != '\0'; cp2++)
			{
				if (strchr(ALL_DELIM, cp2[0]))
					break;
			}

			if ( (cp1[0] == '\0') || (cp2[0] == '\0') || strchr(END_DELIM, cp2[0]) )
			{
				/* Option is not in option list*/
				return -1;
			}

			if (cp2[0] != ',')
				res++;

			len -= (cp2 - cp1) + 1;
			cp1  = cp2 + 1;

		}
		*output = res;
	}

	return 0;
}

char *strtok2(char *srcStr, const char *del, char **ptr)
{
	char	*resStr;
	size_t	srcLen, resLen;

	if (del == NULL)
	{
		del = (char *)"\t ";
	}

	/* Normal Type Of strtok	*/
	if (ptr == NULL)
	{
		return(strtok(srcStr, del));
	}

	if (srcStr && *srcStr)
	{
		srcLen = strlen(srcStr);
		resStr = strtok(srcStr, del);
		resLen = strlen(resStr);
	}
	else
	{
		srcStr = NULL;
		srcLen = 0;
		resStr = NULL;
		resLen = 0;
	}

	if (resStr && ((srcStr+srcLen) > (resStr+resLen)))
		*ptr = (resStr + resLen + 1);
	else
		*ptr = NULL;
	return(resStr);
}

char *strtrim(char *src)
{
	if (src == NULL)
		return src;

	while ((*src == '\t') || (*src == ' '))
	{
		if (*src == '\0') break;
		src++;
	}

	return(src);
}
