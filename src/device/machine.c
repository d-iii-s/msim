
/*
 * Machine
 *
 * Copyright (c) 2000-2008 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "machine.h"

#include "../cpu/processor.h"
#include "../input.h"
#include "../output.h"
#include "../debug/gdb.h"
#include "../env.h"
#include "../check.h"
#include "../utils.h"


/* common variables */
bool tohalt	= false;

char *config_file = 0;

/* debug features */
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;
bool change			= true;
bool interactive		= false;
bool errors			= true;

bool version			= false;

bool remote_gdb			= false;
int remote_gdb_port		= 0;
bool remote_gdb_conn		= false;
bool remote_gdb_step		= false;
bool remote_gdb_one_step	= false;

bool reenter;

uint32_t stepping;
processor_s *focus;
mem_element_s *memlist;
LLList_s *ll_list;
long long msteps;
bool tobreak = false;  /* user break indicator */








static void
machine_user_break( int par)

{
	if (tobreak || interactive)
	{
		mprintf( "Quit\n");
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



/* one machine cycle */
void
machine_step( void)

{
	device_s *dev;

	/* Increase machine cycle counter */
	msteps++;
	
	/* First traverse all the devices which requires processing time every
	 * step. */
	dev = NULL;
	while (dev_next_in_step( &dev))
		dev->type->step( dev);
	
	/* Then, every 4096th cycle traverse all the devices implementing step4
	 * function. */
	if (msteps % 4096 == 0) {
		dev = NULL;
		while (dev_next_in_step4( &dev))
			dev->type->step4( dev);
	}
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


/** Registering current processor for LL-SC tracking.
 */
void
RegisterLL( void)

{
	LLList_s *l;
				
	/* Ignore if already registered. */
	if (ll_list != NULL)
		for (l=ll_list; l; l=l->next)
			if (l->p == pr) return;

	/* Add processor to the link. */
	l = (LLList_s *) xmalloc( sizeof( LLList_s));
	l->p = pr;
	l->next = ll_list;
	ll_list = l;
}


/** Remove current processor from the LL-SC tracking list.
 */
void
UnregisterLL( void)

{
	LLList_s *l, *lo = NULL;

	if (ll_list == NULL)
		return;

	for (l=ll_list; l; lo=l, l=l->next)
		if (l->p == pr)
		{
			if (lo) 
				lo->next = l->next;
			else
				ll_list = l->next;

			free( l);
			break;
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
	PRE( e != NULL);
	
	e->next = memlist;
	memlist = e;
}


/* removes memory element from the list; never fails */
void
mem_unlink( mem_element_s *e)

{
	mem_element_s *ex = memlist, *ex2=0;

	PRE( e != NULL);
	
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
