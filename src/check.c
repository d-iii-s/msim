/*
 * Check routines
 *
 * See the header file check.h for details.
 */

#include <stdio.h>
#include <stdarg.h>
#ifdef RQ_FATAL
#	include <stdlib.h>
#endif

#include "check.h"


#ifdef RQ_DEBUG
void RQ_test( const char *filename, int lineno, const char *func, ...)

{
	int i=0, b;
	va_list va;
	const char *term;

	va_start( va, func);
	do {
		b = va_arg( va, int);
		i++;
	} while (b && b!=RQ_PARM_BRK);
	
	
	if (b != RQ_PARM_BRK)
	{
		do {
			b = va_arg( va, int);
		} while (b != RQ_PARM_BRK);

		term = va_arg( va, const char *);
		fflush( stdout);
		fprintf( stderr, "RQ -- %s(%d): %s() the %d. condition of "
				"\"%s\" does not hold\n",
				filename, lineno, func, i, term);

#ifdef RQ_FATAL
		exit( RQ_FATAL);
#endif
	}
	va_end( va);
}
#endif
