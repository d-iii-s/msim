/*
 * Copyright (c) 2003-2008 Viliam Holub
 * Copyright (c) 2008 Martin Decky
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
#include "cpu/r4000.h"
#include "debug/gdb.h"
#include "device/device.h"
#include "device/machine.h"
#include "assert.h"
#include "env.h"
#include "text.h"
#include "parser.h"
#include "endian.h"
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
	{
		"non-deterministic",
		no_argument,
		0,
		'n'
	},
	{ NULL, 0, NULL, 0 }
};

static void conf_remote_gdb(const char *opt)
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
		
		int c = getopt_long(argc, args, "tVic:hg:n",
		    long_options, &option_index);
		
		if (c == -1)
			break;
		
		switch (c) {
		case 't':
			totrace = true;
			break;
		case 'V':
			printf(txt_version);
			return false;
		case 'i':
			interactive = true;
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
			conf_remote_gdb(optarg);
			break;
		case 'n':
			nondet = true;
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

int main(int argc, char *args[])
{
	init_machine();
	if (parse_cmdline(argc, args)) {
		script();
		go_machine();
	}
	done_machine();
	
	return 0;
}
