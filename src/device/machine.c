/*
 * Copyright (c) 2000-2008 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "machine.h"

#include "../arch/signal.h"
#include "../cpu/processor.h"
#include "../io/input.h"
#include "../io/output.h"
#include "../debug/gdb.h"
#include "../env.h"
#include "../check.h"
#include "../utils.h"


/**< Memory breakpoints */
list_t mem_bps;

/**< Common variables */
bool tohalt = false;
char *config_file = 0;

/**< Debug features */
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;
bool change = true;
bool interactive = false;
bool errors = true;

bool version = false;

bool remote_gdb = false;
int remote_gdb_port = 0;
bool remote_gdb_conn = false;
bool remote_gdb_step = false;
bool remote_gdb_one_step = false;

bool reenter;

uint32_t stepping;
mem_element_s *memlist;
llist_t *ll_list;
static uint64_t msteps;

/**< User break indicator */
bool tobreak = false; 


void init_machine(void)
{
	memlist = 0;
	regname = reg_name[ireg];
	cp0name = cp0_name[ireg];
	cp1name = cp1_name[ireg];
	cp2name = cp2_name[ireg];
	cp3name	= cp3_name[ireg];
	stepping = 0;
	ll_list = NULL;
	msteps = 0;
	
	reenter = false;
	
	input_init();
	input_shadow();
	register_sigint();
	
	list_init(&mem_bps);
}


static void print_statistics(void)
{
	if (msteps == 0)
		return;
	
	mprintf("\nCycles: %" PRIu64 "\n", msteps);
}


void done_machine(void)
{
	input_back();
	print_statistics();
}



/** One machine cycle
 *
 */
void machine_step(void)
{
	device_s *dev;

	/* Increase machine cycle counter */
	msteps++;
	
	/* First traverse all the devices
	   which requires processing time every step */
	dev = NULL;
	while (dev_next_in_step(&dev))
		dev->type->step(dev);
	
	/* Then, every 4096th cycle traverse
	   all the devices implementing step4 function */
	if ((msteps % 4096) == 0) {
		dev = NULL;
		while (dev_next_in_step4(&dev))
			dev->type->step4(dev);
	}
}


/** Main machine cycle
 *
 */
void go_machine(void)
{
	while (!tohalt) {
		if ((remote_gdb) && (!remote_gdb_conn)) {
			remote_gdb_conn = gdb_remote_init();
			if (!remote_gdb_conn)
				interactive = true;
			remote_gdb_step = remote_gdb_conn;
			remote_gdb = remote_gdb_conn;
		}
	
		if ((remote_gdb) && (remote_gdb_conn) && (remote_gdb_step)) {
			remote_gdb_step = false;
			gdb_session(0);
		}
		
		/* Debug - step check */
		if ((stepping) && (!--stepping))
			interactive = true;
		
		/* Interactive mode control */
		if (interactive)
			interactive_control();
	
		/* Step */
		if (!tohalt)
			machine_step();
	}
}


/** Register current processor in LL-SC tracking list
 *
 */
void register_ll(processor_t *pr)
{
	llist_t *l;
					
	/* Ignore if already registered. */
	if (ll_list != NULL)
		for (l = ll_list; l; l = l->next)
			if (l->p == pr)
				return;

	/* Add processor to the list */
	l = (llist_t *) safe_malloc_t(llist_t);
	l->p = pr;
	l->next = ll_list;
	ll_list = l;
}


/** Remove current processor from the LL-SC tracking list
 *
 */
void unregister_ll(processor_t *pr)
{
	llist_t *l, *lo = NULL;

	if (ll_list == NULL)
		return;

	for (l = ll_list; l; lo = l, l = l->next)
		if (l->p == pr) {
			if (lo) 
				lo->next = l->next;
			else
				ll_list = l->next;

			free(l);
			break;
		}
}


/** Memory read
 *
 */
uint32_t mem_read(processor_t *pr, uint32_t addr)
{
	device_s *dev;

	/* Search for memory region */
	mem_element_s *e = memlist;
	mem_element_s *eo = 0;

	for (; e != 0; eo = e, e = e->next)
		if ((addr >= e->start) && (addr < e->start + e->size)) {
			/* Hit - optimize the list */
			if (eo) {
				eo->next = e->next;
				e->next = memlist;
				memlist = e;
			}
			break;
		}

	if (!e) { /* No memory hit */
		uint32_t val = 0xffffffff;
		
		/* List for each device */
		dev = 0;
		while (dev_next(&dev))
			if (dev->type->read)
				dev->type->read(pr, dev, addr, &val);
		
		return val;
	}
	
	/* Check for memory read breakpoints */
	mem_breakpoint_t *mem_bp;
	for_each(mem_bps, mem_bp, mem_breakpoint_t) {
		if ((mem_bp->addr == addr) && (mem_bp->rd)) {
			mprintf("\nDebug: Read from address %#10" PRIx64 "\n\n", mem_bp->addr);
			mem_bp->hits++;
			interactive = true;
			break;
		}
	}

	/* Now there is correct read/write command */
	return convert_uint32_t_endian(*((uint32_t *) &e->mem[addr - e->start]));
}


/** Memory write
 *
 */
void mem_write(processor_t *pr, uint32_t addr, uint32_t val, int size)
{
	device_s *dev;

	/* Search for memory region */
	mem_element_s *e = memlist;
	mem_element_s *eo = 0;

	for (; e != 0; eo = e, e = e->next)
		if ((addr >= e->start) && (addr < e->start + e->size)) {
			/* Hit - optimize the list */
			if (eo) {
				eo->next = e->next;
				e->next = memlist;
				memlist = e;
			}
			break;
		}

	if (!e) { /* No memory hit */
	
		/* List for each device */
		dev = 0;
		while (dev_next(&dev))
			if (dev->type->write) 
				dev->type->write(pr, dev, addr, val);
		
		return;
	}

	/* Writting to ROM? */
	if (!e->writ)
		return;

	/* Now we have the memory write command */

	/* ll control */
	if (ll_list != 0) {
		llist_t *l, *l2, *lo = 0;
		
		for (l = ll_list; l;) {
			if (l->p->lladdr == addr) {
				l->p->llval = false;
				if (lo) 
					lo->next = l->next;
				else
					ll_list = l->next;
				l2 = l;
				l = l->next;
				free(l2);
				continue;
			}
			lo = l;
			l = l->next;
		}
	}

	switch (size) {
	case INT8:
		*((uint8_t *) &e->mem[addr - e->start]) = convert_uint8_t_endian(val);
		break;
	case INT16:
		*((uint16_t *) &e->mem[addr - e->start]) = convert_uint16_t_endian(val);
		break;
	case INT32:
		*((uint32_t *) &e->mem[addr - e->start]) = convert_uint32_t_endian(val);
		break;
	}
	
	/* Check for memory write breakpoints */
	mem_breakpoint_t *mem_bp;
	for_each(mem_bps, mem_bp, mem_breakpoint_t) {
		if ((mem_bp->addr == addr) && (mem_bp->wr)) {
			mprintf("\nDebug: Written to address %#10" PRIx64 "\n\n", mem_bp->addr);
			mem_bp->hits++;
			interactive = true;
			break;
		}
	}
}


/** Add memory element to memory list
 *
 * Never fails.
 *
 */
void mem_link(mem_element_s *e)
{
	PRE(e != NULL);
	
	e->next = memlist;
	memlist = e;
}


/** Remove memory element from the list
 *
 * Never fails.
 *
 */
void mem_unlink(mem_element_s *e)
{
	mem_element_s *ex = memlist, *ex2 = 0;

	PRE(e != NULL);
	
	while ((ex) && (ex != e)) {
		ex2 = ex;
		ex = ex->next;
	}
	
	if (ex) {
		if (ex2)
			ex2->next = e->next;
		else
			memlist = e->next;
	}
}
