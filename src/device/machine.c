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
#include <assert.h>

#include "machine.h"

#include "../arch/signal.h"
#include "../cpu/processor.h"
#include "../io/input.h"
#include "../io/output.h"
#include "../debug/gdb.h"
#include "../debug/breakpoint.h"
#include "../device/dcpu.h"
#include "../env.h"
#include "../check.h"
#include "../utils.h"
#include "../fault.h"

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

/** Set by the user from command line to enable debugging globaly. */
bool remote_gdb = false;

/** Port for debugger tcp/ip connection. */
int remote_gdb_port = 0;

/** Indicate whether the connection to the debugger was opened. */
bool remote_gdb_conn = false;

/** The simulator will read a message from the debugger, if this flag is set */
bool remote_gdb_listen = false;

/** Indicate that the debugger has sent a step command */
bool remote_gdb_step = false;

bool reenter;

uint32_t stepping = 0;

/** Head of the list with memory regions */
mem_element_s *memlist = NULL;
llist_t *ll_list;
static uint64_t msteps;

/**< User break indicator */
bool tobreak = false; 


void init_machine(void)
{
	memlist = NULL;
	regname = reg_name[ireg];
	cp0name = cp0_name[ireg];
	cp1name = cp1_name[ireg];
	cp2name = cp2_name[ireg];
	cp3name = cp3_name[ireg];
	stepping = 0;
	ll_list = NULL;
	msteps = 0;
	
	reenter = false;
	
	input_init();
	input_shadow();
	register_sigint();
	
	dev_init_framework();
	memory_breakpoint_init_framework();
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
	while (dev_next(&dev, DEVICE_FILTER_STEP))
		dev->type->step(dev);
	
	/* Then, every 4096th cycle traverse
	   all the devices implementing step4 function */
	if ((msteps % 4096) == 0) {
		dev = NULL;
		while (dev_next(&dev, DEVICE_FILTER_STEP4))
			dev->type->step4(dev);
	}
}

/** Try to run gdb communication.
 *
 * @return True if the connection was opened.
 *
 */
static bool gdb_startup(void) {
	if (dcpu_find_no(0) == NULL) {
		error("Cannot debug without configured processor.");
		return false;
	}
	
	remote_gdb_conn = gdb_remote_init();
	
	if (!remote_gdb_conn)
		return false;
	
	/*
	 * In case of succesfull opening the debugging session will start
	 * in stopped state and in case of unsuccesfull opening the
	 * connection will not be initialized again.
	 */
	remote_gdb_listen = remote_gdb_conn;
	remote_gdb = remote_gdb_conn;
	
	return true;
}

/** Main machine cycle
 *
 */
void go_machine(void)
{
	while (!tohalt) {
		/*
		 * Check for code breakpoints. Interactive
		 * or gdb flags will be set if a breakpoint
		 * is hit.
		 */
		breakpoint_check_for_code_breakpoints();
		
		/*
		 * Open connection to the debugger, if the debugging is
		 * allowed and the connection has not been opened yet.
		 */
		if ((remote_gdb) && (!remote_gdb_conn))
			interactive = !gdb_startup();
		
		/*
		 * Read messages from the debugger, if we are stopped for
		 * debugging. The read is blocking.
		 */
		if ((remote_gdb) && (remote_gdb_conn) && (remote_gdb_listen)) {
			remote_gdb_listen = false;
			gdb_session();
		}
		
		/* Debug - step check */
		if (stepping > 0) {
			stepping--;
			
			if (stepping == 0)
				interactive = true;
		}
		
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
	if (ll_list != NULL) {
		for (l = ll_list; l; l = l->next) {
			if (l->p == pr)
				return;
		}
	}
	
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
	
	for (l = ll_list; l; lo = l, l = l->next) {
		if (l->p == pr) {
			if (lo) 
				lo->next = l->next;
			else
				ll_list = l->next;
			
			free(l);
			break;
		}
	}
}

static mem_element_s* find_memory_region(ptr_t addr)
{
	mem_element_s *current_region = memlist;
	mem_element_s *previous_region = NULL;
	
	/* Search for memory region for given address */
	while (current_region != NULL) {
		ptr_t region_end = current_region->start + current_region->size;
		
		if ((addr >= current_region->start) && (addr < region_end)) {
			/* Hit - optimize the list, put the current region to the first place */
			if (previous_region != NULL) {
				previous_region->next = current_region->next;
				current_region->next = memlist;
				memlist = current_region;
			}
			
			break;
		}
		
		/* Move to the next region */
		previous_region = current_region;
		current_region = current_region->next;
	}
	
	return current_region;
}


/** Memory read
 *
 * Read bytes from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param pr             Processor which wants to read.
 * @param addr           Address of memory to be read.
 * @param size           Number of bytes to read. Should be one of INT8,
 *                       INT16 or INT32.
 * @param protected_read If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory trimmed to given size
 *         or the default memory value, if the address is not valid.
 *
 */
uint32_t mem_read(processor_t *pr, ptr_t addr, size_t size,
    bool protected_read)
{
	mem_element_s *region = find_memory_region(addr);
	
	/*
	 * No region found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (region == NULL) { 
		uint32_t val = DEFAULT_MEMORY_VALUE32;
		
		/* List for each device */
		device_s *dev = NULL;
		while (dev_next(&dev, DEVICE_FILTER_ALL))
			if (dev->type->read)
				dev->type->read(pr, dev, addr, &val);
		
		return val;
	}
	
	/* Check for memory read breakpoints */
	if (protected_read)
		memory_breakpoint_check_for_breakpoint(addr, ACCESS_READ);
	
	unsigned char* value_address = &region->mem[addr - region->start];
	
	/* Now there is correct read/write command */
	switch (size) {
	case INT8:
		return convert_uint8_t_endian(*((uint8_t *) value_address));
	case INT16:
		return convert_uint16_t_endian(*((uint16_t *) value_address));
	case INT32:
		return convert_uint32_t_endian(*((uint32_t *) value_address));
	default:
		assert(false);
		return DEFAULT_MEMORY_VALUE32;
	}
}


/** Memory write
 *
 * Write the given data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param pr              Processor, which wants to write to the memory.
 * @param addr            Address of the memory.
 * @param val             Data to be written.
 * @param size            One of INT8, INT16 or INT32.
 * @param protected_write False to allow writing to ROM memory and ignore
 *                        the memory breakpoints check.
 *
 * @return False, if there is no configured memory region and device for
 *         given address or the memory is ROM with protected_write parameter
 *         set to true.
 *
 */
bool mem_write(processor_t *pr, uint32_t addr, uint32_t val, size_t size,
    bool protected_write)
{
	mem_element_s *region = find_memory_region(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (region == NULL) {
		
		/* List for each device */
		device_s *dev = NULL;
		bool written = false;
		
		while (dev_next(&dev, DEVICE_FILTER_ALL))
			if (dev->type->write) {
				dev->type->write(pr, dev, addr, val);
				written = true;
			}
		
		return written;
	}
	
	/* Writting to ROM? */
	if ((!region->writable) && (protected_write))
		return false;
	
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
	
	/* Check for memory write breakpoints */
	if (protected_write)
		memory_breakpoint_check_for_breakpoint(addr, ACCESS_WRITE);
	
	unsigned char* value_address = &region->mem[addr - region->start];
	
	switch (size) {
	case INT8:
		*((uint8_t *) value_address) = convert_uint8_t_endian(val);
		break;
	case INT16:
		*((uint16_t *) value_address) = convert_uint16_t_endian(val);
		break;
	case INT32:
		*((uint32_t *) value_address) = convert_uint32_t_endian(val);
		break;
	default:
		assert(false);
	}
	
	return true;
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
