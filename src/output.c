/*
 * MSIM Output management
 * 
 * Copyright (c) 2003,2004 Viliam Holub
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


#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "output.h"

#include "check.h"
#include "parser.h"

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
dprintf( const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];

	RQ( fmt != NULL);

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
dprintf2( const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];

	RQ( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	fprintf( output, "%s", dbuf);
}


/** Counts occupation space of the string.
 *
 */
int
string_space( const char *str)

{
	int r;

	for (r=0; *str; r++, str++)
		if (*str == '\t')
			r += 8-r%8 -1;

	return r;
}


/** Prints the message with breaking tags. 
 *
 * nl	string which will be displayed on new lines
 */
void
dprintf_btag( const char *nl, const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];
	int ncur, acur;
	char *s, *so;
	char x;
	int sw = screen_width;
	bool newl = false;

	RQ( fmt != NULL);

	if (!nl)
		nl = "";
	ncur = string_space( nl);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	s = dbuf;
	for (x=*s; x; s++)
	{
		so = s;
		for (; *s && *s!=TBRK_C; s++) ;

		x = *s; *s = 0;

		acur = string_space( so);
		scr_cur += acur;
		if (newl && scr_cur > sw-1)
		{
			dprintf2( "\n%s", nl);
			scr_cur = ncur +acur;
		}
		
		dprintf2( "%s", so);
		if (*so)
			if (so[ strlen( so)-1] == '\n')
				scr_cur = 0;
		newl = true;
	}
}


/** dprintf_n - Aka dprintf but with limited output length.
 */
void
dprintf_n( int n, const char *fmt, ...)

{
	char buf[ 256];
	va_list ap;

	RQ( fmt != NULL);

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
dprintf_text( const char *nl, const char *fmt, ...)

{
	char buf[ BUF_SIZ];
	char buf2[ 2*BUF_SIZ];
	va_list ap;
	char *s, *s2;

	RQ( fmt != NULL);

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

	dprintf_btag( nl, "%s", buf2);
}


/** Prints the error message
 */
void
dprintf_err( const char *fmt, ...)

{
	char buf[ BUF_SIZ];
	va_list ap;
	size_t len;

	RQ( fmt != NULL);

	if (lineno != -1)
		len = sprintf( buf, "%d: ", lineno);
	else
		len = 0;
	buf[ len] = '\0';

	va_start( ap, fmt);
	vsnprintf( buf+len, BUF_SIZ-len, fmt, ap);
	va_end( ap);

	dprintf_text( NULL, "%s", buf);
}
