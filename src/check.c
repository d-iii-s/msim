/*
 * Pre/post conditions
 * 2004 Viliam Holub
 *
 * See the header file check.h for additional information.
 *
 * The auxiliary function checks conditions and displays an error message.
 * Regarding to the specified parameter the program may be halted.
 *
 * Feel free to modify this code to fulfill your needs.
 */

#include <stdio.h>
#include <stdarg.h>
#ifdef RQ_COLOR
#	include <unistd.h>
#endif
#ifdef RQ_FATAL
#	include <stdlib.h>
#endif

#include "check.h"

#define COL_FILENAME		0
#define COL_LINENO		1
#define COL_FUNCNAME		2
#define COL_TEXT		3
#define COL_CONDITION		4
#define COL_NORMAL		5
#define COL_CONDNO		6
#define COL_END			7
char *colors[ COL_END][ 2] = {
	"", "\x1b\x5b\x31\x3b\x33\x36\x6d",
	"", "\x1b\x5b\x33\x34\x6d",
	"", "\x1b\x5b\x31\x3b\x33\x31\x6d",
	"", "\x1b\x5b\x30\x6d",
	"", "\x1b\x5b\x31\x3b\x33\x31\x6d",
	"", "\x1b\x5b\x30\x6d",
	"", "\x1b\x5b\x31\x3b\x33\x31\x6d",
};


#ifdef RQ_DEBUG

#define RQ_COND_NO_SIZE		128

void RQ_test( const char *pre, const char *filename, int lineno,
		const char *func, const char *term, ...)

{
	int i=0, c=0, b;
	va_list va;

	// check conditions
	va_start( va, term);
	do {
		b = va_arg( va, int);
		i++;
		c++;
	} while (b && b != RQ_PARM_BRK);
	
	
	// if there is one which does not hold...
	if (b != RQ_PARM_BRK)
	{
		int cl = 0;
		char cond_no[ RQ_COND_NO_SIZE];

		fflush( stdout);

#ifdef RQ_COLOR
		if (isatty( fileno( stderr)))
			cl = 1;
#endif
		cond_no[ 0] = '\0';
		if (c != 1)
			snprintf( cond_no, RQ_COND_NO_SIZE, "the %s%d.%s ",
				colors[ COL_CONDNO][ cl], i,
				colors[ COL_TEXT][ cl]);

		fprintf( stderr, "\n%s%s%s(%s%d%s): in %s%s()%s %s%scondition %s"
			"%s\"%s\"%s does not hold%s\n",
			colors[ COL_FILENAME][ cl], filename,
			colors[ COL_TEXT][ cl],
			colors[ COL_LINENO][ cl], lineno,
			colors[ COL_TEXT][ cl],
			colors[ COL_FUNCNAME][ cl], func,
			colors[ COL_TEXT][ cl],
			cond_no, pre,
			cond_no[ 0] ? "of " : "",
			colors[ COL_CONDITION][ cl], term,
			colors[ COL_TEXT][ cl],
			colors[ COL_NORMAL][ cl]);
#ifdef RQ_FATAL
		exit( RQ_FATAL);
#endif
	}
	va_end( va);
}
#endif
