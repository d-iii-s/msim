/*
 * Input routines
 *
 * Copyright (c) 2004 Viliam Holub
 */

#include "../config.h"

#include <termios.h>
#include <unistd.h>

#include "parser.h"
// for tobreak and reenter variable
#include "machine.h"
#include "cmd.h"
#include "utils.h"

#include "input.h"
#include "output.h"

/*
 * Include readline library of present
 */
#if defined HAVE_LIBREADLINE && defined HAVE_READLINE_READLINE_H && defined HAVE_READLINE_HISTORY_H
#	define _HAVE_READLINE_OK_
#endif

#ifdef _HAVE_READLINE_OK_
#	include <readline/readline.h>
#	include <readline/history.h>
#endif



/*
 * A shared variable for tab completion.
 */
char *par_text;

bool input_term;		// is the std input terminal
struct termios tio_shadow;
struct termios tio_inter;
struct termios tio_old;


/** Terminal and readline init.
 */
void
input_init( void)

{
	/* terminal init */
	input_term = !!isatty( 0);
	
	if (!input_term)
		return;
	
	tcgetattr( 0, &tio_shadow);
	 
	tio_old = tio_shadow;
	tio_inter = tio_shadow;
	tio_shadow.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio_shadow.c_lflag &= ~(IUCLC);
#endif
	tio_shadow.c_cc[ VMIN] = 1;
	tio_shadow.c_cc[ VTIME] = 0;
	
	tio_inter.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio_inter.c_lflag &= ~(IUCLC);
#endif
	tio_inter.c_cc[ VMIN] = 1;
	tio_inter.c_cc[ VTIME] = 0;

#ifdef _HAVE_READLINE_OK_
	/* readline init */
	rl_readline_name = "msim";
	rl_attempted_completion_function = msim_completion;
#endif
}


void
input_inter( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio_inter);
}


void
input_shadow( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio_shadow);
}


void
input_back( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio_old);
}


/** hint_generator - Generates all possible words suitable for completion.
 */
char *
hint_generator( const char *input, int level)

{
	char *s;
	const void *data = NULL;
	static parm_link_s *pl = NULL;
	static gen_f generator;

	if (level == 0)
	{
		parm_delete( pl);
		pl = parm_parse( par_text);
		if (!pl)
			return NULL;
		parm_check_end( pl, par_text);
		generator = NULL;

		find_system_generator( &pl, &generator, &data);

		if (!generator)
			return NULL;
	}

	s = generator( pl, data, level);

	return s;
}


/** msim_completion -- Tries to complete the user input.
 */
char **
msim_completion( const char *text, int start, int end)

{
	char **result;

	//printf( "\ncompletion: test: %s, start: %d, end: %d, line_buffer: %s\n", text, start, end, rl_line_buffer);

	par_text = (char *)xmalloc( end+1);
	strncpy( par_text, rl_line_buffer, end);
	par_text[ end] = '\0';

	rl_attempted_completion_over = 1;

	result = rl_completion_matches( /*par_text*/"", hint_generator);
	free( par_text);

	//rl_attempted_completion_function = qwert;

	return result;
}


#ifndef _HAVE_READLINE_OK_

static char *
readline( char *s)

{
	char cmml_buf[ 256];
	int i=0;
	int c;

	input_inter();

	printf( "%s", s);
	s = (char *) cmml_buf;

	do {
		c = fgetc( stdin);
		s[ i] = c;

		if ((c == 127) || (c == '\b'))
		{
			if (i != 0)
			{
				i--;
				printf( "\b \b");
				fflush( stdout);
			}
			continue;
		}
		
		fputc( c, stdout);
		fflush( stdout);
		
		if (i<254)
			i++;
	} while (c != '\n');

	s[ i] = 0;

	input_shadow();

	return xstrdup( cmml_buf);
}
#endif


/** Interactive mode control.
 */
void
interactive_control( void)

{
	char *commline;
	
	//input_back();
	//input_inter();

	tobreak = false;

	if (reenter)
	{
		printf( "\n");
		reenter = false;
	}

	stepping = 0;
	
	while (interactive)
	{
#ifdef _HAVE_READLINE_OK_
		input_back();
		commline = readline( "[msim] ");
		input_shadow();

		if (!commline)
		{
			// user break in readline
			dprintf( "Quit\n");
			input_back();
			exit( 1);
		}
		
		if (*commline)
			add_history( commline);
#else
		commline = readline( "[msim] ");
#endif
		
		if (*commline)
			interpret( commline);
		else
			interpret( "s");

		free( commline);
	}
		
	//input_shadow();
}
