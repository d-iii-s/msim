/*
 * Check routines.
 *
 * Copyright (c) 2004 Viliam Holub
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "check.h"


#ifdef RQ_DEBUG
void RQ_fail( const char *filename, int lineno, const char *func, ...)

{
	int i=0, b;
	va_list va;
	const char *term;

	va_start( va, func);
	do {
		b = va_arg( va, int);
		i++;
	} while (b && b!=-2);
	
	
	if (b != -2)
	{
		do {
			b = va_arg( va, int);
		} while (b != -2);
		term = va_arg( va, const char *);
		fprintf( stderr, "RQ -- %s(%d): %s() the %d. condition of \"%s\" does not hold\n",
			filename, lineno, func, i, term);
	}
	va_end( va);
}
#endif
