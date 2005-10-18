/*
 * fault.c
 * fault handlers
 * Copyright (c) 2003 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "fault.h"


/* message dump to stderr */
void
error( const char *fmt, ...)

{
	va_list ap;
	
	fflush( stdout);

	fprintf( stderr, "%s: ", PACKAGE);

	if (!(!fmt || !*fmt))
	{
	        va_start( ap, fmt);
		vfprintf( stderr, fmt, ap);
		va_end( ap);
	}

	fprintf( stderr, "\n");
}


/* prints message to stderr and exit */
void
die( int ex, const char *fmt, ...)

{
	va_list ap;
	
	fflush( stdout);


	if (!(!fmt || !*fmt))
	{
		fprintf( stderr, "%s: ", PACKAGE);
		
	        va_start( ap, fmt);
		vfprintf( stderr, fmt, ap);
		va_end( ap);
		
		fprintf( stderr, "\n");
	}
	

	input_back();
	
	exit( ex);
}
		

void
io_error( const char *filename)

{
	if (filename)
		error( "%s: %s", filename, strerror( errno));
	else
		error( "%s", strerror( errno));
}
		

void
io_die( int n, const char *filename)

{
	die( n, "%s: %s", filename, strerror( errno));
}
