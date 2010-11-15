/*
 * Copyright (c) 2003-2008 Viliam Holub
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS R4000 32bit system simulator
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "cpu/cpu.h"
#include "debug/gdb.h"
#include "device/device.h"
#include "device/machine.h"
#include "io/output.h"
#include "text.h"
#include "parser.h"
#include "endi.h"
#include "cmd.h"
#include "fault.h"
#include "utils.h"

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
	{ NULL, 0, NULL, 0 }
};

static void conf_remote_gdb(const char *opt)
{
	char *endp;
	long int port_no;
	
	port_no = strtol(opt, &endp, 0);
	if (!endp)
		die(ERR_PARM, "Port number expected.");
	
	if ((port_no < 0) || (port_no > 65534))
		die(ERR_PARM, "Invalid port number.");
	
	remote_gdb = true;
	remote_gdb_port = port_no;
}


static void parse_cmdline(int argc, char *args[])
{
	int c;

	opterr = 0;

	while (1) {
		int option_index = 0;
	
		c = getopt_long( argc, args, "tVic:hg:",
			long_options, &option_index);
	
		if (c == -1)
			break;

		switch (c) {
		case 't':
			totrace = true;
			break;
		case 'V':
			mprintf(txt_version);
			done_machine();
			exit(0);
		case 'i':
			interactive = true;
			break;
		case 'c':
			if (config_file)
				free(config_file);
			config_file = safe_strdup(optarg);
			break;
		case 'h':
			mprintf(txt_version);
			mprintf(txt_help);
			done_machine();
			exit(0);
		case 'g':
			conf_remote_gdb(optarg);
			break;
		case '?':
			die(ERR_PARM, "Unknown parameter or argument required\n");
		default:
			die(ERR_PARM, "Unknown parameter \"%c\"\n", optopt);
		}
	}
	
	if (optind < argc)
		die(ERR_PARM, "Unexpected arguments.\n");
}


int main(int argc, char *args[])
{
	output_init();
	init_machine();
	parse_cmdline(argc, args);
	
	script();
	go_machine();
	done_machine();

	return 0;
}
