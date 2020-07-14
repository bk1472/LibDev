#ifdef  __CMD_SHELL_C__
/*-----------------------------------------------------------------------------
 * command:
 * 			color
------------------------------------------------------------------------------*/
static void debug_TestColor(void)
{
	dbgprint("^k^Foreground black ");
	dbgprint("^r^Foreground red   ");
	dbgprint("^g^Foreground green ");
	dbgprint("^y^Foreground yellow Background black");
	dbgprint("^b^Foreground blue  ");
	dbgprint("^p^Foreground purple");
	dbgprint("^c^Foreground cyan  ");
	dbgprint("^a^Foreground black  ");
	dbgprint("^K^Foreground white  Background black ");
	dbgprint("^R^Foreground white  Background red   ");
	dbgprint("^G^Foreground black  Background green ");
	dbgprint("^Y^Foreground black  Background yellow");
	dbgprint("^B^Foreground white  Background blue  ");
	dbgprint("^P^Foreground white  Background purple");
	dbgprint("^C^Foreground black  Background cyan  ");
	dbgprint("^A^Foreground white  Background black  ");
	return;
}

/*-----------------------------------------------------------------------------
 * command:
 * 			dummy
------------------------------------------------------------------------------*/
static void debug_Dummy (void)
{
	extern void print_stack(void);

	print_stack();

	return;
}
/*-----------------------------------------------------------------------------
 * command:
 * 			crash
------------------------------------------------------------------------------*/
int crash_func2(uint64_t* reg)
{
	uint64_t* _crash = reg;


	_crash[0] = 0x12345678;
	return 0;
}

int crash_func1(void)
{
	uint64_t* reg = NULL;

	return crash_func2(reg);
}

static void debug_Crash (void)
{
	int a = crash_func1();

	rprint1n("^r^ call crash function:%d", a);
	return;
}

/*-----------------------------------------------------------------------------
 * command:
 * 			sh
------------------------------------------------------------------------------*/
void debug_os_shell(int argc, char **argv)
{
	char sh_buf[70];

	sh_buf[0] = 0;

	if(!argc) snprintf(sh_buf, 70, "%s", "/bin/bash");
	else
	{
		int cnt = argc;

		while(cnt)
		{
			snprintf(sh_buf, 70, "%s %s", sh_buf, *argv);
			cnt--;
			argv++;
		}
	}
	system(sh_buf);
	return;
}

/*-----------------------------------------------------------------------------
 * command:
 * 			md
------------------------------------------------------------------------------*/
static uint64_t debug_GetAddress(char *arg, bool dsm_mod)
{
	char		*remain;
	int			sign = -1;
	uint64_t	addr;
	bool		_valid;

	if (1             )	{ strtok2(arg, "+", &remain); sign =  1; }
	if (remain == NULL)	{ strtok2(arg, "-", &remain); sign = -1; }

	addr = strtoul2(arg, NULL, 0);

	if (addr == 0)
	{
		addr = findSymByName(arg);
	}

	if      (sign ==  1) addr += strtoul2(remain, NULL, 0);
	else if (sign == -1) addr -= strtoul2(remain, NULL, 0);

	if(dsm_mod)
		_valid = isValidPc(addr);
	else
		_valid = isValidAddr(addr);

	if (_valid == false)
	{
		rprint1n("^r^Address[0x%016lx] is invalid, set to default", addr);
		addr = (uint64_t)__START;
	}

	return(addr);
}

static char* debug_GetSymbolName(uint64_t addr, char *retName, size_t size, char *defName, int precision, int bRef)
{
	char		*symName = defName;
	uint64_t	sym_addr;
	char		dmgBuf[80];

	sym_addr = findDmgSymByAddr(addr, &symName, &dmgBuf[0], 80);
	if (sym_addr != 0)
	{
		if ( bRef || (addr == sym_addr) )
			snprintf(retName, size, "%*s+0x%04lx", precision, symName, addr - sym_addr);
		else
			snprintf(retName, size, "%*s+0x%04lx", precision, "", addr - sym_addr);
		symName = retName;
	}
	else if (defName == NULL)
	{
		snprintf(retName, size, "0x%08lx", addr);
		symName = retName;
	}
	else
	{
		symName = defName;
	}
	return(symName);
}


static void debug_MemoryDump(char **argv)
{
	static uint64_t	addr = __START, size = 0x80;
	char			*symName, symNameBuf[64];

	if (argv)
	{
		if (*argv) addr = debug_GetAddress(*argv++, false);
		if (*argv) size = strtoul2(*argv++, NULL, 0);
		addr &= ~0x7LL;
	}

	symName = debug_GetSymbolName(addr, symNameBuf, 64, "MemoryDump", 0, 1);
	rprint1n("addr:0x%016lx, size:%ld", addr, size);
	hexdump(symName, (void *)addr, size);

	addr += size;
	return;
}

/*-----------------------------------------------------------------------------
 * command:
 * 			dsm
------------------------------------------------------------------------------*/
static uint64_t dsmaddr[3] = {__START};

static void putaddr(uint64_t addr)
{
	int i;
	for(i = 2; i >= 0; i--)
	{
		if(i == 0)
			dsmaddr[i] = addr;
		else
			dsmaddr[i] = dsmaddr[i-1];
	}
}

static uint64_t getaddr(int index)
{
	return dsmaddr[index];
}

void debug_Disassemble(char **argv)
{
	static uint32_t	count = 9;
	uint64_t		addr;
	int				i, inc;

	addr = getaddr(0);

	if (argv)
	{
		if (*argv)
		{
			char *arg0 = *argv++;

			if (strcmp(arg0, "..") == 0)
			{
				addr = getaddr(1);
			}
			else
			{
				addr = debug_GetAddress(arg0, true);
			}
		}
		if (*argv) count = strtoul2(*argv++, NULL, 0);
		addr &= ~0x3;
	}

	rprint0n("\n\t[    Symbol Name   | Address|Hexcode         |       Assembly code       ]\n");

	for (i = 0; i < count; i++)
	{
		char	symNameBuf[50];
		rprint0n("\t%-19.19s] ", debug_GetSymbolName(addr, symNameBuf, 32, "      None", -12, (i == 0)));

		inc = dsmInst((inst_t *)addr, (uint64_t)addr, rprint0n);

		addr += inc;
		if(inc == 0)
			break;
	}

	putaddr(addr);
	return;
}

/*-----------------------------------------------------------------------------
 * command:
 * 			pt
------------------------------------------------------------------------------*/
static void debug_toggle_time(void)
{
	extern bool OptTMInfo_Print;

	rprint0n("Time info of \"printf\" %s -> ", (OptTMInfo_Print)?"Enable":"Disable");
	OptTMInfo_Print = OptTMInfo_Print?false:true;

	rprint1n("^y^%s", (OptTMInfo_Print)?"Enable":"Disable");
}

/*-----------------------------------------------------------------------------
 * command:
 * 			task
------------------------------------------------------------------------------*/
static void debug_ShowThread(char* name)
{
	int	taskId = -1;

	if(name != NULL)
	{
		if ( (('a' <= name[0]) && (name[0] <= 'z')) ||
		     (('A' <= name[0]) && (name[0] <= 'Z')) )
		{
			taskId = get_tsk_id_by_name((const char*)name);
		}
		else
		{
			if(name[1] == 'x' || name[1] == 'X')
				sscanf(name, "%x", &taskId);
			else
				sscanf(name, "%d", &taskId);
			if (taskId == 0) taskId = get_current();
		}
	}

	thread_info_print(taskId);
}

/*-----------------------------------------------------------------------------
 * command:
 * 			exp
------------------------------------------------------------------------------*/
#include <math.h>

static double ExpAdjustment[256] =
{
	1.040389835,	1.039159306,	1.037945888,	1.036749401,
	1.035569671,	1.034406528,	1.033259801,	1.032129324,
	1.031014933,	1.029916467,	1.028833767,	1.027766676,
	1.02671504,		1.025678708,	1.02465753,		1.023651359,
	1.022660049,	1.021683458,	1.020721446,	1.019773873,
	1.018840604,	1.017921503,	1.017016438,	1.016125279,
	1.015247897,	1.014384165,	1.013533958,	1.012697153,
	1.011873629,	1.011063266,	1.010265947,	1.009481555,
	1.008709975,	1.007951096,	1.007204805,	1.006470993,
	1.005749552,	1.005040376,	1.004343358,	1.003658397,
	1.002985389,	1.002324233,	1.001674831,	1.001037085,
	1.000410897,	0.999796173,	0.999192819,	0.998600742,
	0.998019851,	0.997450055,	0.996891266,	0.996343396,
	0.995806358,	0.995280068,	0.99476444, 	0.994259393,
	0.993764844,	0.993280711,	0.992806917,	0.992343381,
	0.991890026,	0.991446776,	0.991013555,	0.990590289,
	0.990176903,	0.989773325,	0.989379484,	0.988995309,
	0.988620729,	0.988255677,	0.987900083,	0.987553882,
	0.987217006,	0.98688939, 	0.98657097, 	0.986261682,
	0.985961463,	0.985670251,	0.985387985,	0.985114604,
	0.984850048,	0.984594259,	0.984347178,	0.984108748,
	0.983878911,	0.983657613,	0.983444797,	0.983240409,
	0.983044394,	0.982856701,	0.982677276,	0.982506066,
	0.982343022,	0.982188091,	0.982041225,	0.981902373,
	0.981771487,	0.981648519,	0.981533421,	0.981426146,
	0.981326648,	0.98123488, 	0.981150798,	0.981074356,
	0.981005511,	0.980944219,	0.980890437,	0.980844122,
	0.980805232,	0.980773726,	0.980749562,	0.9807327,
	0.9807231,  	0.980720722,	0.980725528,	0.980737478,
	0.980756534,	0.98078266, 	0.980815817,	0.980855968,
	0.980903079,	0.980955475,	0.981017942,	0.981085714,
	0.981160303,	0.981241675,	0.981329796,	0.981424634,
	0.981526154,	0.981634325,	0.981749114,	0.981870489,
	0.981998419,	0.982132873,	0.98227382, 	0.982421229,
	0.982575072,	0.982735318,	0.982901937,	0.983074902,
	0.983254183,	0.983439752,	0.983631582,	0.983829644,
	0.984033912,	0.984244358,	0.984460956,	0.984683681,
	0.984912505,	0.985147403,	0.985388349,	0.98563532,
	0.98588829, 	0.986147234,	0.986412128,	0.986682949,
	0.986959673,	0.987242277,	0.987530737,	0.987825031,
	0.988125136,	0.98843103, 	0.988742691,	0.989060098,
	0.989383229,	0.989712063,	0.990046579,	0.990386756,
	0.990732574,	0.991084012,	0.991441052,	0.991803672,
	0.992171854,	0.992545578,	0.992924825,	0.993309578,
	0.993699816,	0.994095522,	0.994496677,	0.994903265,
	0.995315266,	0.995732665,	0.996155442,	0.996583582,
	0.997017068,	0.997455883,	0.99790001, 	0.998349434,
	0.998804138,	0.999264107,	0.999729325,	1.000199776,
	1.000675446,	1.001156319,	1.001642381,	1.002133617,
	1.002630011,	1.003131551,	1.003638222,	1.00415001,
	1.004666901,	1.005188881,	1.005715938,	1.006248058,
	1.006785227,	1.007327434,	1.007874665,	1.008426907,
	1.008984149,	1.009546377,	1.010113581,	1.010685747,
	1.011262865,	1.011844922,	1.012431907,	1.013023808,
	1.013620615,	1.014222317,	1.014828902,	1.01544036,
	1.016056681,	1.016677853,	1.017303866,	1.017934711,
	1.018570378,	1.019210855,	1.019856135,	1.020506206,
	1.02116106, 	1.021820687,	1.022485078,	1.023154224,
	1.023828116,	1.024506745,	1.025190103,	1.02587818,
	1.026570969,	1.027268461,	1.027970647,	1.02867752,
	1.029389072,	1.030114973,	1.030826088,	1.03155163,
	1.032281819,	1.03301665, 	1.033756114,	1.034500204,
	1.035248913,	1.036002235,	1.036760162,	1.037522688,
	1.038289806,	1.039061509,	1.039837792,	1.040618648
};

#if 0
static long DoubleToLongBits(double value)
{
	return *((long *)&value);
}
#endif
static double Int64BitsToDouble(long value)
{
	return *((double*)&value);
}


static double fast_exp(double x)
{
	int		index;
	long	tmp = (long) (1512775 * x + 1072632447);

	index = (int)(tmp >> 12) & 0xFF;

	return (Int64BitsToDouble(tmp << 32) * ExpAdjustment[index]);
}

static void debug_ExpTest (void)
{
	double		d = 0.0;
	uint64_t 	st, ct;

	st = xlibc_read_msticks();
	for (d = 1.0; d <= 99.0; d+= 1.0)
	{
		dbgprint("^y^%02f => exp(%f)", d, exp(d));
	}
	ct = xlibc_read_msticks();
	rprint1n("^c^Total elapsed time: %7ld ms", ct - st);

	st = xlibc_read_msticks();
	for (d = 1.0; d <= 99.0; d+= 1.0)
	{
		dbgprint("^g^%02f => fast_exp(%f)", d, fast_exp(d));
	}
	ct = xlibc_read_msticks();
	rprint1n("^p^Total elapsed time: %7ld ms", ct - st);
}

#endif//__CMD_SHELL_C__
