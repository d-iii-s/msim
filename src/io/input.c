/*
 * Copyright (c) 2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Input routines
 *
 */

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../arch/console.h"
#include "../parser.h"
#include "../device/machine.h"
#include "../cmd.h"
#include "../utils.h"

#include "input.h"
#include "output.h"

/**< Shared variable for tab completion */
char *par_text;

/**< Is the standard input a terminal? */
bool input_term;

struct termios tio_shadow;
struct termios tio_inter;
struct termios tio_old;


/** Terminal and readline initialization
 *
 */
void input_init(void)
{
	input_term = !!isatty(0);
	
	if (!input_term)
		return;
	
	(void) tcgetattr(0, &tio_shadow);
	 
	tio_old = tio_shadow;
	tio_inter = tio_shadow;
	tio_shadow.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio_shadow.c_lflag &= ~(IUCLC);
#endif
	tio_shadow.c_cc[VMIN] = 1;
	tio_shadow.c_cc[VTIME] = 0;
	
	tio_inter.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio_inter.c_lflag &= ~(IUCLC);
#endif
	tio_inter.c_cc[VMIN] = 1;
	tio_inter.c_cc[VTIME] = 0;

	rl_readline_name = "msim";
	rl_attempted_completion_function = msim_completion;
}


void input_inter(void)
{
	if (input_term)
		(void) tcsetattr(0, TCSANOW, &tio_inter);
}


void input_shadow( void)
{
	if (input_term)
		(void) tcsetattr(0, TCSANOW, &tio_shadow);
}


void input_back( void)
{
	if (input_term)
		(void) tcsetattr(0, TCSANOW, &tio_old);
}


/** Generate all possible words suitable for completion
 *
 */
char *hint_generator(const char *input, int level)
{
	char *s;
	static const void *data;
	static parm_link_s *pl = NULL;
	static gen_f generator;

	if (level == 0) {
		parm_delete(pl);
		pl = parm_parse(par_text);
		if (!pl)
			return NULL;
		parm_check_end(pl, par_text);
		generator = NULL;

		data = NULL;
		find_system_generator(&pl, &generator, &data);

		if (!generator)
			return NULL;
	}

	s = generator(pl, data, level);

	return s;
}


/** Try to complete the user input
 *
 */
char **msim_completion(const char *text, int start, int end)
{
	char **result;

	par_text = (char *) safe_malloc(end + 1);
	strncpy(par_text, rl_line_buffer, end);
	par_text[end] = '\0';

	rl_attempted_completion_over = 1;

	result = rl_completion_matches("", hint_generator);
	free(par_text);

	return result;
}


/** Interactive mode control
 *
 */
void interactive_control(void)
{
	char *commline;
	
	tobreak = false;

	if (reenter) {
		mprintf("\n");
		reenter = false;
	}

	stepping = 0;
	
	while (interactive) {
		input_back();
		commline = readline("[msim] ");
		input_shadow();

		if (!commline) {
			/* User break in readline */
			mprintf("Quit\n");
			input_back();
			exit(1);
		}
		
		if (*commline) {
			add_history(commline);
			interpret(commline);
		} else
			interpret("s");

		free(commline);
	}
}
