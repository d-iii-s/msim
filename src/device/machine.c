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
#include "../cpu/r4000.h"
#include "../debug/gdb.h"
#include "../debug/breakpoint.h"
#include "../device/dcpu.h"
#include "../input.h"
#include "../env.h"
#include "../assert.h"
#include "../utils.h"
#include "../fault.h"

/** Common variables */
bool tohalt = false;
bool change = true;
bool interactive = false;
bool version = false;
bool reenter = false;

/** User break indicator */
bool tobreak = false;

char *config_file = NULL;
uint64_t stepping = 0;

/** Debug features */
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;

/** Set by the user from command line to enable debugging globaly. */
bool remote_gdb = false;

/** Port for debugger tcp/ip connection. */
unsigned int remote_gdb_port = 0;

/** Indicate whether the connection to the debugger was opened. */
bool remote_gdb_conn = false;

/** The simulator will read a message from the debugger, if this flag is set */
bool remote_gdb_listen = false;

/** Indicate that the debugger has sent a step command */
bool remote_gdb_step = false;

/** Memory areas */
list_t physmem_areas;
list_t sc_list;

static uint64_t msteps = 0;

void init_machine(void)
{
	regname = reg_name[ireg];
	cp0name = cp0_name[ireg];
	cp1name = cp1_name[ireg];
	cp2name = cp2_name[ireg];
	cp3name = cp3_name[ireg];
	
	list_init(&physmem_areas);
	list_init(&sc_list);
	
	input_init();
	input_shadow();
	register_sigint();
	
	dev_init_framework();
	breakpoint_init_framework();
}

static void print_statistics(void)
{
	if (msteps == 0)
		return;
	
	printf("\nCycles: %" PRIu64 "\n", msteps);
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
	device_t *dev = NULL;
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
		error("Cannot debug without any processor");
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
void register_sc(cpu_t *cpu)
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
void unregister_sc(cpu_t *cpu)
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
