/*
 * MSIM Output management
 * 
 * Copyright (c) 2003-2005 Viliam Holub
 *
 *
 * We must take a special care with output messages. Some types of messages
 * often exceeds the with of the screen and just simple splitting to more
 * lines may cause unexpected output format.
 *
 * Thus we provide a set of output functions which behaviour depends on
 * the type of information they belongs to.
 *
 * Regular text messages are buffered and lines are splitted at word
 * boundaries.
 *
 * This approach works fine in most cases.
 *
 * XXX
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "output.h"

#include "check.h"
#include "parser.h"
#include "cline.h"

/*
 * The output buffer size
 */
#define OUTPUT_BUF_SIZE	256
#define DBUF_SIZE	4096


/*
 * Output file descriptor.
 */
FILE *output;


volatile int screen_width;
int buf_end;
char output_buf[ OUTPUT_BUF_SIZE];

int scr_cur;


/** output_init - Initializes the output management.
 */
void
output_init( void)
	
{
	output = stdout;
	buf_end = 0;
	scr_cur = 0;
	
	if (isatty( 1))
		screen_width = 79; /* ? */
	else
		screen_width = 79;
}



/** Prints the message on the screen.
 */
void
mprintf( const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];

	PRE( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	fprintf( output, "%s", dbuf);
	scr_cur = 0;
}


/** Prints the message on the screen.
 *
 * The cursor pointer is not modified.
 */
void
mprintf2( const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];

	PRE( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	fprintf( output, "%s", dbuf);
}


/** Counts a string space occupation on the screen.
 *
 * The only interesting character is '\t' which has to be counted carefully.
 */
int
string_space( const char *str)

{
	int r;

	PRE( str != NULL);

	for (r=0; *str; r++, str++)
		if (*str == '\t')
			r += 8-r%8 -1;

	return r;
}


/** Prints the message with breaking tags. 
 *
 * The function prints the formatted text specified. Additionally, it breaks
 * the line on a special character BTAG, if the text could exceed the screen
 * width. On a new line, string "nl" is printed.
 *
 * There should not be the end-of-line '\n' character except the end of fmt
 * string.
 *
 * nl	string which will be displayed on new lines
 * fmt	a printf-like text format
 */
void
mprintf_btag( const char *nl, const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];
	int ncur, acur;
	char *s;
	char x;
	int sw = screen_width;
	bool next_line = false;

	PRE( fmt != NULL);

	if (!nl)
		nl = "";
	ncur = string_space( nl);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	// line break test
	if (scr_cur > sw-1)
	{
		mprintf2( "\n%s", nl);
		scr_cur = ncur;
		next_line = true;
	}

	s = dbuf;
	for (x=*s; x; s++)
	{
		char *so = s; // keeps the substring start

		// remove TBRK prefix
		while (*s && *s == TBRK_C)
			s++;

		// find the end of the substring
		for (; *s && *s!=TBRK_C; s++) ;

		// save the "type" of the end, set the string terminator
		x = *s; *s = 0;

		// calculate the substring length
		acur = string_space( so);
		scr_cur += acur;

		// line break test
		if (scr_cur > sw-1 && !next_line)
		{
			mprintf2( "\n%s", nl);
			scr_cur = ncur +acur;
			next_line = true;
		}
		next_line = false;
		
		// print the substring
		if (x)
			mprintf2( "%s", so);
		else
		{
			mprintf2( "%s", so);

			if (*(s-1) == '\n')
				scr_cur = 0;
		}

	}
}


/** Aka mprintf but with limited output length.
 *
 * Maximum string length is 256.
 */
void
mprintf_n( int n, const char *fmt, ...)

{
	char buf[ 256];
	va_list ap;

	PRE( fmt != NULL);

	if (n > 255)
		n = 255;
	
	va_start( ap, fmt);
	vsnprintf( buf, n+1, fmt, ap);
	va_end( ap);

	fprintf( output, "%s", buf);
	scr_cur = 0;
}


#define BUF_SIZ	256

/** Prints the text.
 */
void
mprintf_text( const char *nl, const char *fmt, ...)

{
	char buf[ BUF_SIZ];
	char buf2[ 2*BUF_SIZ];
	va_list ap;
	char *s, *s2;

	PRE( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( buf, BUF_SIZ, fmt, ap);
	va_end( ap);

	s = buf;
	s2 = buf2;
	
	for (; *s; s++, s2++)
	{
		if (*s == ' ' || *s == '\n')
		{
			*s2 = *s;
			s2++;
			*s2 = TBRK_C;
			continue;
		}

		*s2 = *s;
	}
	*s2 = '\0';

	mprintf_btag( nl, "%s", buf2);
}


/** Prints the error message
 */
void
mprintf_err( const char *fmt, ...)

{
	char buf[ BUF_SIZ];
	va_list ap;
	size_t len;

	PRE( fmt != NULL);

	if (lineno != -1)
		len = sprintf( buf, "%d: ", lineno);
	else
		len = 0;
	buf[ len] = '\0';

	va_start( ap, fmt);
	vsnprintf( buf+len, BUF_SIZ-len, fmt, ap);
	va_end( ap);

	mprintf_text( NULL, "%s", buf);
}
