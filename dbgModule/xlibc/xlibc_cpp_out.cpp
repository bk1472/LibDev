#include	<iostream>
#include	<ostream>
#include	<fstream>
#include	"xlibc_output.h"

using namespace std;
#ifndef PRINT_LVL_ALL
 #define PRINT_LVL_ALL	2
 #define PRINT_LVL_DBG	1
 #define PRINT_LVL_NON	0
#endif

static std::streambuf	*backup = nullptr;
static std::streambuf	*psbuf  = nullptr;
static std::ofstream	filestr;

extern "C"
int xlibc_setCoutStat(int _new)
{
	static int _firstcall = 0;

	if (_firstcall == 0)
	{
		backup = std::cout.rdbuf();
		filestr.open("/dev/null");
		psbuf = filestr.rdbuf();

		_firstcall = 1;
	}

	if (_new > PRINT_LVL_ALL)
		_new = PRINT_LVL_ALL;
	if (_new < PRINT_LVL_NON)
		_new = PRINT_LVL_NON;

	if (_new == PRINT_LVL_ALL)
		std::cout.rdbuf(backup);
	else
		std::cout.rdbuf(psbuf);

	return _new;
}
