
/*
 * Machine
 *
 * Copyright (c) 2000-2004 Viliam Holub
 */

#include "../config.h"

#if defined HAVE_LIBREADLINE && defined HAVE_READLINE_READLINE_H && defined HAVE_READLINE_HISTORY_H
#	define _HAVE_READLINE_OK_
#endif

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#ifdef _HAVE_READLINE_OK_
#	include <readline/readline.h>
#	include <readline/history.h>
#endif

#include "text.h"
#include "machine.h"
#include "processor.h"
#include "mcons.h"
#include "mtypes.h"
#include "debug.h"
#include "cmd.h"
#include "output.h"
#include "gdb.h"
#include "env.h"
#include "check.h"
#include "parser.h"
#include "utils.h"


/* common variables */
bool tohalt	= false;

char *config_file = 0;

/* debug features */
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;
bool change	= true;
bool interactive= false;
bool errors	= true;
bool breakpoint;
bool script_stat;
bool halt_on_error	= false;

bool version	= false;

bool remote_gdb		= false;
int remote_gdb_port	= 0;
bool remote_gdb_conn	= false;
bool remote_gdb_step	= false;
bool remote_gdb_one_step	= false;

bool reenter;

int breakpointaddr;
uint32_t stepping;
processor_s *focus;
mem_element_s *memlist;
LLList_s *ll_list;
long long msteps;
bool tobreak = false;  /* user break indicator */

bool input_term;
struct termios tio;
struct termios tio_inter;
struct termios tio_old;



static void
input_init( void)

{
	input_term = !!isatty( 0);
	
	if (!input_term)
		return;
	
	tcgetattr( 0, &tio);
	 
	tio_old = tio;
	tio_inter = tio;
	tio.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio.c_lflag &= ~(IUCLC);
#endif
	tio.c_cc[ VMIN] = 1;
	tio.c_cc[ VTIME] = 0;
	
	tio_inter.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
	tio_inter.c_lflag &= ~(IUCLC);
#endif
	tio_inter.c_cc[ VMIN] = 1;
	tio_inter.c_cc[ VTIME] = 0;
}



static void
input_inter( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio_inter);
}


static void
input_shadow( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio);
}


void
input_back( void)

{
	if (input_term)
		tcsetattr( 0, TCSANOW, &tio_old);
}


static void
machine_user_break( int par)

{
	if (tobreak || interactive)
	{
		dprintf( "Quit\n");
		input_back();
		exit( 1);
	}
	
	tobreak = true;
	if (!interactive)
		reenter = true;
	interactive = true;
}


static void
register_sigint( void)

{
	struct sigaction act;
	
	act.sa_handler = machine_user_break;
	sigemptyset( &act.sa_mask);
	act.sa_flags = 0;
	sigaction( SIGINT, &act, 0);
} 


/*
 * A shared variable for tab completion.
 */
char *par_text;


/** hint_generator - Generates all possible words suitable for completion.
 */
static char *
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
static char **
msim_completion( const char *text, int start, int end)

{
	char **result;

//	printf( "\ncompletion: test: %s, start: %d, end: %d, line_buffer: %s\n", text, start, end, rl_line_buffer);

	par_text = (char *)xmalloc( end+1);
	strncpy( par_text, rl_line_buffer, end);
	par_text[ end] = '\0';

	result = rl_completion_matches( /*par_text*/"", hint_generator);
	free( par_text);

	return result;
}


void
init_machine( void)

{
	memlist		= 0;
	regname		= RegName[ ireg];
	cp0name		= cp0_name[ ireg];
	cp1name		= cp1_name[ ireg];
	cp2name		= cp2_name[ ireg];
	cp3name		= cp3_name[ ireg];
	stepping	= 0;
	focus		= 0;
	ll_list		= 0;
	msteps		= 0;
	
	reenter		= false;
	
	input_init();
	input_shadow();
	register_sigint();

#ifdef _HAVE_READLINE_OK_
	/* readline init */
	rl_readline_name = "msim";
	rl_attempted_completion_function = msim_completion;
#endif
}


static void
print_statistics( void)

{
	if (msteps == 0)
		return;
	
	printf( "\nCycles: %lld\n", msteps);
	// XXX
/*
	if (tobreak) printf( "User break!\n\n");
		else printf( "Machine halted!\n\n");
*/
}


void
done_machine( void)

{
	input_back();
	print_statistics();
}

#ifndef _HAVE_READLINE_OK_

char *readline( char *s)

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


/* interactive mode control */
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
			interpret( commline, -1);
		else
			interpret( "s", -1);

		free( commline);
	}
		
	//input_shadow();
}


/* one machine cycle */
void
machine_step( void)

{
	device_s *dev = 0;

	msteps++;
	
	while (dev_next( &dev))
		if (dev->type->step)
			dev->type->step( dev);
}


/*
 * go_machine
 * main cycle
 */
void
go_machine( void)

{
	while (!tohalt)
	{
		if (remote_gdb && !remote_gdb_conn)
		{
			remote_gdb_conn = gdb_remote_init();
			if (!remote_gdb_conn)
				interactive = true;
			remote_gdb_step = remote_gdb_conn;
			remote_gdb = remote_gdb_conn;
		}
	
		if (remote_gdb && remote_gdb_conn && remote_gdb_step)
		{
			remote_gdb_step = false;
			gdb_session( 0);
		}
		
		/* debug - step check */
		if (stepping && !--stepping)
			interactive = true;
		
		/* interactive mode control */
		if (interactive)
			interactive_control();
	
		/* step */
		if (!tohalt)
			machine_step();
	}
}



/* 
 * registering ll
 * default parameter is pr from processor
 */
void
RegisterLL( void)

{
	LLList_s *l;
				
	if (ll_list != 0)
		for (l=ll_list; l; l=l->next)
			if (l->p == pr) return;

	l = (LLList_s *) xmalloc( sizeof( LLList_s));
	l->p = pr;
	l->next = ll_list;
	ll_list = l;
}


void
UnregisterLL( void)

{
	LLList_s *l, *lo=0;
	
	if (ll_list != 0)
		for (l=ll_list; l; lo=l, l=l->next)
			if (l->p == pr)
			{
				if (lo) 
					lo->next = l->next;
				else
					ll_list = l->next;

				free( l);
				return;
				
			}
}


/*
 * mem_read
 *
 */
uint32_t
mem_read( uint32_t addr)

{
	device_s *dev;

	/* searching for memory region */
	mem_element_s *e = memlist;
	mem_element_s *eo = 0;


	for ( ; e != 0; eo=e, e=e->next)
		if ((addr >= e->start) && (addr < e->start + e->size))
		{
			/* hit! - optimize the list */
			if (eo)
			{
				eo->next = e->next;
				e->next = memlist;
				memlist = e;
			}

			break;
		}

	if (!e) /* no memory hit */
	
	{
		uint32_t val = 0xffffffff;
		/* list for each device */
		dev = 0;
		while (dev_next( &dev))
			if (dev->type->read)
				dev->type->read( dev, addr, &val);
		
		return val;
	}

	/* now there is correct read/write command */
	return convert_uint32_t_endian( *((uint32_t *) &e->mem[ addr -e->start]));
}


/*
 * mem_write
 *
 */
void
mem_write( uint32_t addr, uint32_t val, int size)

{
	device_s *dev;


	/* searching for memory region */
	mem_element_s *e = memlist;
	mem_element_s *eo = 0;


	for ( ; e != 0; eo=e, e=e->next)
		if ((addr >= e->start) && (addr < e->start+e->size))
		{
			/* hit! - optimize the list */
			if (eo)
			{
				eo->next = e->next;
				e->next = memlist;
				memlist = e;
			}

			break;
		}

	if (!e) /* no memory hit */
	
	{
		/* now list for each device */
		dev = 0;
		while (dev_next( &dev))
			if (dev->type->write) 
				dev->type->write( dev, addr, val);
		
		return;
	}

	/* writting to ROM ? */
	if (!e->writ)
		return;

	/* now we have the memory write command */

	/* ll control */
	if (ll_list != 0)
	{
		LLList_s *l, *l2, *lo = 0;
		
		for (l=ll_list; l;)
		{
			if (l->p->lladdr == addr)
			{
				l->p->llval = false;
				if (lo) 
					lo->next = l->next;
				else
					ll_list = l->next;
				l2 = l;
				l = l->next;
				free( l2);
				continue;
			}
			lo = l;
			l = l->next;
		}
	}

	switch (size)
	{
		case INT8:
			*((uint8_t *)&e->mem[ addr -e->start]) = convert_uint8_t_endian( val);
			break;
		case INT16:
			*((uint16_t *)&e->mem[ addr -e->start]) = convert_uint16_t_endian( val);
			break;
		case INT32:
			*((uint32_t *)&e->mem[ addr -e->start]) = convert_uint32_t_endian( val);
			break;
	}
}



/* memory control */

/* adds memory element to memory list; never fails */
void
mem_link( mem_element_s *e)

{
	REQUIRED( e != NULL);
	
	e->next = memlist;
	memlist = e;
}


/* removes memory element from the list; never fails */
void
mem_unlink( mem_element_s *e)

{
	mem_element_s *ex = memlist, *ex2=0;

	REQUIRED( e != NULL);
	
	while (ex && (ex != e))
	{
		ex2 = ex;
		ex = ex->next;
	}
	
	if (ex)
	{
		if (ex2)
			ex2->next = e->next;
		else
			memlist = e->next;
	}
}
