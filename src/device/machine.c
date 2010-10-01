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

/** Common variables */
bool tohalt = false;
bool change = true;
bool interactive = false;
bool errors = true;
bool version = false;
bool reenter = false;

/** User break indicator */
bool tobreak = false;

char *config_file = NULL;
uint32_t stepping = 0;

/** Debug features */
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;

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

/** Memory areas */
list_t mem_areas;
list_t sc_list;

static uint64_t msteps = 0;

void init_machine(void)
{
	regname = reg_name[ireg];
	cp0name = cp0_name[ireg];
	cp1name = cp1_name[ireg];
	cp2name = cp2_name[ireg];
	cp3name = cp3_name[ireg];
	
	list_init(&mem_areas);
	list_init(&sc_list);
	
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
	/* Increase machine cycle counter */
	msteps++;
	
	/* First traverse all the devices
	   which requires processing time every step */
	device_s *dev = NULL;
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
void register_sc(processor_t *cpu)
{
	/* Ignore if already registered. */
	sc_item_t *sc_item;
	
	for_each(sc_list, sc_item, sc_item_t) {
		if (sc_item->cpu == cpu)
			return;
	}
	
	sc_item = safe_malloc_t(sc_item_t);
	item_init(&sc_item->item);
	sc_item->cpu = cpu;
	list_append(&sc_list, &sc_item->item);
}

/** Remove current processor from the LL-SC tracking list
 *
 */
void unregister_sc(processor_t *cpu)
{
	sc_item_t *sc_item;
	
	for_each(sc_list, sc_item, sc_item_t) {
		if (sc_item->cpu == cpu) {
			list_remove(&sc_list, &sc_item->item);
			safe_free(sc_item);
			break;
		}
	}
}

static inline mem_area_t* find_mem_area(ptr_t addr)
{
	mem_area_t *area;
	
	for_each(mem_areas, area, mem_area_t) {
		ptr_t area_start = area->start;
		ptr_t area_end = area_start + area->size;
		
		if ((addr >= area_start) && (addr < area_end))
			return area;
	}
	
	return NULL;
}

/** Find an activated memory breakpoint
 *
 * Find an activated memory breakpoint which would be hit for specified
 * memory address and access conditions.
 *
 * @param addr         Address, where the breakpoint can be hit.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 * @return Found breakpoint structure or NULL if there is not any.
 *
 */
static inline mem_breakpoint_t *memory_breakpoint_find(ptr_t addr,
    access_t access_type)
{
	mem_breakpoint_t *breakpoint;
	
	for_each(memory_breakpoints, breakpoint, mem_breakpoint_t) {
		if (breakpoint->addr != addr)
			continue;
		
		if ((access_type & breakpoint->access_flags) != 0)
			return breakpoint;
	}
	
	return NULL;
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
	mem_area_t *area = find_mem_area(addr);
	
	/*
	 * No memory area found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (area == NULL) {
		uint32_t val = DEFAULT_MEMORY_VALUE;
		
		/* List for each device */
		device_s *dev = NULL;
		while (dev_next(&dev, DEVICE_FILTER_ALL))
			if (dev->type->read)
				dev->type->read(pr, dev, addr, &val);
		
		return val;
	}
	
	/* Check for memory read breakpoints */
	if (protected_read) {
		mem_breakpoint_t *breakpoint =
		    memory_breakpoint_find(addr, ACCESS_READ);
		
		if (breakpoint != NULL)
			memory_breakpoint_hit(breakpoint, ACCESS_READ);
	}
	
	unsigned char *value_ptr = &area->data[addr - area->start];
	
	/* Now there is correct read/write command */
	switch (size) {
	case BITS_8:
		return convert_uint8_t_endian(*((uint8_t *) value_ptr));
	case BITS_16:
		return convert_uint16_t_endian(*((uint16_t *) value_ptr));
	case BITS_32:
		return convert_uint32_t_endian(*((uint32_t *) value_ptr));
	default:
		assert(false);
		return DEFAULT_MEMORY_VALUE;
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
	mem_area_t *area = find_mem_area(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (area == NULL) {
		
		/* List for each device */
		device_s *dev = NULL;
		bool written = false;
		
		while (dev_next(&dev, DEVICE_FILTER_ALL)) {
			if (dev->type->write) {
				dev->type->write(pr, dev, addr, val);
				written = true;
			}
		}
		
		return written;
	}
	
	/* Writting to ROM? */
	if ((!area->writable) && (protected_write))
		return false;
	
	/* Now we have the memory write command */
	
	/* Load Linked and Store Conditional control */
	sc_item_t *sc_item = (sc_item_t *) sc_list.head;
	
	while (sc_item != NULL) {
		processor_t *sc_cpu = sc_item->cpu;
		
		if (sc_cpu->lladdr == addr) {
			sc_cpu->llbit = false;
			
			sc_item_t *tmp = sc_item;
			sc_item = (sc_item_t *) sc_item->item.next;
			
			list_remove(&sc_list, &tmp->item);
			safe_free(tmp);
		} else
			sc_item = (sc_item_t *) sc_item->item.next;
	}
	
	/* Check for memory write breakpoints */
	if (protected_write) {
		mem_breakpoint_t *breakpoint =
		    memory_breakpoint_find(addr, ACCESS_WRITE);
		
		if (breakpoint != NULL)
			memory_breakpoint_hit(breakpoint, ACCESS_WRITE);
	}
	
	unsigned char *value_ptr = &area->data[addr - area->start];
	
	switch (size) {
	case BITS_8:
		*((uint8_t *) value_ptr) = convert_uint8_t_endian(val);
		break;
	case BITS_16:
		*((uint16_t *) value_ptr) = convert_uint16_t_endian(val);
		break;
	case BITS_32:
		*((uint32_t *) value_ptr) = convert_uint32_t_endian(val);
		break;
	default:
		assert(false);
	}
	
	return true;
}
