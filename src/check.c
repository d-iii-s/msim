/*
 * Pre/post conditions
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
void RQ_test( const char *pre, const char *filename, int lineno,
		const char *func, ...)

{
	int i=0, c=0, b;
	va_list va;
	const char *term;

	va_start( va, func);
	do {
		b = va_arg( va, int);
		i++;
		c++;
	} while (b && b != RQ_PARM_BRK);
	
	
	if (b != RQ_PARM_BRK)
	{
		do {
			b = va_arg( va, int);
			c++;
		} while (b != RQ_PARM_BRK);

		term = va_arg( va, const char *);
		fflush( stdout);
		if (c==2)
			fprintf( stderr, "\n%s(%d): in %s() %scondition "
				"\"%s\" does not hold\n",
				filename, lineno, func, pre, term);
		else
			fprintf( stderr, "\n%s(%d): in %s() the %d. %scondition of "
				"\"%s\" does not hold\n",
				filename, lineno, func, i, pre, term);

#ifdef RQ_FATAL
		exit( RQ_FATAL);
#endif
	}
	va_end( va);
}
#endif
