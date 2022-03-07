/*
 * Copyright (c) 2003-2008 Viliam Holub
 * Copyright (c) 2008-2011 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS system simulator
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include "arch/signal.h"
#include "device/cpu/mips_r4000/cpu.h"
#include "device/cpu/riscv_rv32ima/cpu.h"
#include "debug/gdb.h"
#include "device/dcpu.h"
#include "device/device.h"
#include "assert.h"
#include "cmd.h"
#include "endian.h"
#include "env.h"
#include "fault.h"
#include "input.h"
#include "parser.h"
#include "text.h"
#include "utils.h"

/** Debugging register names */
char **regname;
char **cp0name;
char **cp1name;
char **cp2name;
char **cp3name;

/** Configuration file name */
char *config_file = NULL;

/** Enable remote GDB debugging globally */
bool remote_gdb = false;

/** TCP port for GDB listening socket */
unsigned int remote_gdb_port = 0;

/** Remote GDB conection indication */
bool remote_gdb_conn = false;

/** Ready for remote GDB command */
bool remote_gdb_listen = false;

/** Remote GDB stepping */
bool remote_gdb_step = false;

/** Enable non-deterministic behaviour */
bool machine_nondet = false;

/** Trace instructions */
bool machine_trace = false;

/** Halt the simulation */
bool machine_halt = false;

/** Break the simulation */
bool machine_break = false;

/** Interactive mode */
bool machine_interactive = false;

/** Print newline on entering interactive mode */
bool machine_newline = false;

/** Undefined instruction silent exception */
bool machine_undefined = false;

/** Allow MSIM-specific instructions. */
bool machine_specific_instructions = true;

/** Allow XINT even when terminal is not available. */
bool machine_allow_interactive_without_tty = false;

/**
 * Number of steps to run before switching
 * to interactive mode. Zero means infinite.
 */
uint64_t stepping = 0;

/** SC-LL tracking */
list_t sc_list;

/** Total number of machine steps completed */
static uint64_t steps = 0;

/** Command line options */
static struct option long_options[] = {
	{
		"trace",
		no_argument,
		0,
		't'
	},
	{
		"version",
		no_argument,
		0,
		'V'
	},
	{
		"interactive",
		no_argument,
		0,
		'i'
	},
	{
		"allow-xint-without-tty",
		no_argument,
		0,
		'I'
	},
	{
		"config",
		required_argument,
		0,
		'c'
	},
	{
		"help",
		no_argument,
		0,
		'h'
	},
	{
		"remote-gdb",
		required_argument,
		0,
		'g'
	},
	{
		"non-deterministic",
		no_argument,
		0,
		'n'
	},
	{
		"no-extra-instructions",
		no_argument,
		0,
		'X'
	},
	{ NULL, 0, NULL, 0 }
};

static void setup_remote_gdb(const char *opt)
{
	ASSERT(opt != NULL);
	
	char *endp;
	long int port_no;
	
	port_no = strtol(opt, &endp, 0);
	if (!endp)
		die(ERR_PARM, "Port number expected");
	
	if ((port_no < 0) || (port_no > 65534))
		die(ERR_PARM, "Invalid port number");
	
	remote_gdb = true;
	remote_gdb_port = port_no;
}

static bool parse_cmdline(int argc, char *args[])
{
	opterr = 0;
	
	while (true) {
		int option_index = 0;
		
		int c = getopt_long(argc, args, "tVic:hg:nXI",
		    long_options, &option_index);
		
		if (c == -1)
			break;
		
		switch (c) {
		case 't':
			machine_trace = true;
			break;
		case 'V':
			printf(txt_version);
			return false;
		case 'i':
			machine_interactive = true;
			break;
		case 'I':
			machine_allow_interactive_without_tty = true;
			break;
		case 'c':
			if (config_file)
				safe_free(config_file);
			config_file = safe_strdup(optarg);
			break;
		case 'h':
			printf(txt_version);
			printf(txt_help);
			return false;
		case 'g':
			setup_remote_gdb(optarg);
			break;
		case 'n':
			machine_nondet = true;
			break;
		case 'X':
			machine_specific_instructions = false;
			break;
		case '?':
			die(ERR_PARM, "Unknown parameter or argument required");
		default:
			die(ERR_PARM, "Unknown parameter \"%c\"", optopt);
		}
	}
	
	if (optind < argc)
		die(ERR_PARM, "Unexpected arguments");
	
	return true;
}

/** Try to startup remote GDB communication.
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

/** Run 4096 machine cycles
 *
 */
static void machine_step(void)
{
	/* Execute device cycles */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_STEP))
		dev->type->step(dev);
	
	/* Increase machine cycle counter */
	steps++;
	
	/* Every 4096th cycle execute
	   the step4k device functions */
	if ((steps % 4096) == 0) {
		dev = NULL;
		while (dev_next(&dev, DEVICE_FILTER_STEP4K))
			dev->type->step4k(dev);
	}
}

/** Main simulator loop
 *
 */
static void machine_run(void)
{
	while (!machine_halt) {
		/*
		 * Check for code breakpoints. Interactive
		 * or gdb flags will be set if a breakpoint
		 * is hit.
		 */
		breakpoint_check_for_code_breakpoints();
		
		/*
		 * If the remote GDB debugging is allowed and the
		 * connection has not been opened yet, then wait
		 * for the connection from the remote GDB.
		 */
		
		if ((remote_gdb) && (!remote_gdb_conn))
			machine_interactive = !gdb_startup();
		
		/*
		 * If the simulation was stopped due to the remote
		 * GDB debugging session, then read a command from
		 * the remote GDB. The read is blocking.
		 */
		if ((remote_gdb) && (remote_gdb_conn) && (remote_gdb_listen)) {
			remote_gdb_listen = false;
			gdb_session();
		}
		
		/* Stepping check */
		if (stepping > 0) {
			stepping--;
			
			if (stepping == 0)
				machine_interactive = true;
		}
		
		/* Interactive mode control */
		if (machine_interactive)
			interactive_control();
		
		/*
		 * Continue with the simulation
		 */
		if (!machine_halt)
			machine_step();
	}
}

int main(int argc, char *args[])
{
	/*
	 * Initialization
	 */
	regname = reg_name[ireg];
	cp0name = cp0_name[ireg];
	cp1name = cp1_name[ireg];
	cp2name = cp2_name[ireg];
	cp3name = cp3_name[ireg];
	
	input_init();
	input_shadow();
	register_signal_handlers();
	
	/*
	 * Run-time configuration
	 */
	if (!parse_cmdline(argc, args)) {
		input_back();
		return 0;
	}
	
	script();
	
	if (machine_interactive) {
		alert("MSIM %s", VERSION);
		alert("Entering interactive mode, type `help' for help.");
	}

	/*
	 * Main simulation loop
	 */
	machine_run();
	
	/*
	 * Finalization
	 */
	input_back();
	if (steps > 0)
		printf("\nCycles: %" PRIu64 "\n", steps);
	
	return 0;
}
