/*
 * Verbose assert
 * 2004 Viliam Holub
 *
 * See the header file check.h for additional infomation.
 *
 * The auxiliary function checking conditions and displaying error messages.
 * Regarding to the specified parameter the program may be halted.
 *
 * Feel free to modify this code to fulfill your needs.
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
	} while (b && b != RQ_PARM_BRK);
	
	
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
