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

#include "output.h"

#include "check.h"

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


/** output_init - Initializes the output management.
 */
void
output_init( void)
	
{
	output = stdout;
	buf_end = 0;
	
	if (isatty( 1))
		screen_width = 79; /* ? */
	else
		screen_width = 79;
}


/** dprint_buffer - Prints the buffer on the screen with right width.
 *
 * SE The buffer is changed.
 * RQ The buffer have to be write-accessible.
 */
static void
dprint_buffer( char *buf)

{
	int i;
	char *c, *co, *co2;

	REQUIRED( buf != NULL);

	c = co = buf;

	do {
		for (co=c, i=0; *c && i<screen_width-1; c++, i++) ;

		if (!*c)
		{
			fprintf( output, "%s", co);
			return;
		}
		co2 = c;

		/* find the last white character */
		for (; i && isspace( c); i--, c--) ;

		if (i == 0)
		{
			char t;

			c = co2;
			t = *c;
			*c = '\0';
			fprintf( output, "%s", co);
			*c = t;
		}
		else
		{
			*c = '\0';
			fprintf( output, "%s\n", co);
			c++;
		}
	} while (*c);
}


/** Prints the message on the screen.
 */
void
dprintf( const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];

	REQUIRED( fmt != NULL);

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
 * Assuming the cursor is on the left. 
 *
 * nl	string which will be displayed on new lines
 */
dprintf_btag( const char *nl, const char *fmt, ...)

{
	va_list ap;
	char dbuf[ DBUF_SIZE];
	int cur = 0;
	char *s, *so;
	char x, ncur;
	int sw = screen_width;

	REQUIRED( fmt != NULL);

	if (!nl)
		nl = "";
	ncur = string_space( nl);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	s = dbuf;
	do {
		so = s;
		for (; *s && *s!=TBRK_C; s++) ;
		if (cur+string_space( so) > sw-1)
		{
			dprintf( "\n%s", nl);
			cur = ncur;
		}
		x = *s;
		*s = '\0';
		
		dprintf( "%s", *so);
	} while (x);
}


/** dprintf_n - Aka dprintf but with limited output length.
 */
void
dprintf_n( int n, const char *fmt, ...)

{
	char buf[ 256];
	va_list ap;

	REQUIRED( fmt != NULL);

	if (n > 255)
		n = 255;
	
	va_start( ap, fmt);
	vsnprintf( buf, n+1, fmt, ap);
	va_end( ap);

	fprintf( output, "%s", buf);
}


/** dprintf_text - Prints the regular text.
 *
 * The text is printed as a one line. 
 *
 * RQ fmt should not contain the line delimiter '\n'.
 */
void
dprintf_text( const char *fmt, ...)

{
	size_t len;
	va_list ap;
	char dbuf[ DBUF_SIZE];

	REQUIRED( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE-1, fmt, ap);
	va_end( ap);

	len = strlen( dbuf);
	dbuf[ len  ] = '\n';
	dbuf[ len+1] = '\0';
	dprint_buffer( dbuf);
}


/** new_line - Prints a new line of comments.
 */
/*
static void
new_line( int gap_size)

{
	dprintf( "\n");
	while (gap_size--)
		dprintf( " ");
	dprintf( "# ");
}
*/


/** dprintf_asm - Prints the disassembler text.
 *
 * The disassembler text consists of two main part - the disassembled
 * instruction and an optional debugging comment. These parts are
 * splitted by the '#' character.
 * If the disassembled part does not fit in the screen. We have to print
 * comments on a new line.
 *
 * RQ fmt should not contain the line delimiter '\n'.
 */
/*
void
dprintf_asm( const char *fmt, ...)

{
	int i, o;
	bool first;
	const char *c, *co;
	va_list ap;
	char dbuf[ DBUF_SIZE+1];

	REQUIRED( fmt != NULL);

	va_start( ap, fmt);
	vsnprintf( dbuf, DBUF_SIZE, fmt, ap);
	va_end( ap);

	// search for the '#' character
	for (i=0, c=dbuf; *c && *c!='#'; c++)
		i++;

	// print the disassembly part
	if (*c && *(c+1))
	{
		*c++ = 0;
		dprintf( "%s", dbuf);
	}
	else
	{
		*c = 0;
		dprintf( "%s", dbuf);
		return;
	}

	// print the debug part
	if (i+3 > screen_width)
	{
		i = 8;
		new_line( i);
	}
	first = true;
	
	do {
		// get the size of a partial debug message
		for (o=0, co=c; *c && *c!=','; c++)
			o++;
		if (o == 0)
			break;

		if (*c)
			*c++ = '\0';

		if (i+c+3 > screen_width)

		if (!first)
			dprintf( ", ");
		dprintf( "%s", co);
		i += o;
	} while (true);
}
*/
