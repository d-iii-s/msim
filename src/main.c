/*
 * Computer system simulator
 * 
 * Copyright (c) Viliam Holub 2000-2004
 */


#include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <string.h>

#include "text.h"
#include "mtypes.h"
#include "device.h"
#include "processor.h"
#include "machine.h"
#include "parser.h"
#include "endi.h"
#include "cmd.h"
#include "output.h"
#include "gdb.h"
#include "fault.h"
#include "utils.h"


bool remote_gdb_flag = false;

/*
 * Command line options
 */

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
	{ "trace",		no_argument,		0, 't'},
	{ "version",		no_argument,		0, 'V'},
	{ "interactive",	no_argument,		0, 'i'},
	{ "config",		required_argument,	0, 'c'},
	{ "help", 		no_argument,		0, 'h'},
	{ "remote-gdb",		required_argument,	0, 'r'},
	{}
};
#endif


static void
conf_remote_gdb( const char *opt)

{
	char *endp;
	long int port_no;
	
	port_no = strtol( opt, &endp, 0);
	if (!endp)
		die( 3, "Port number expected.");
	
	if (port_no < 0 || port_no > 65534)
		die( 3, "Invalid port number.");
	
	remote_gdb = true;
	remote_gdb_port = port_no;
}


void
parse_cmdline( int argc, char *args[])

{
	int c;
	
	opterr = 0;
	
	while (1)
	{
		int option_index = 0;
		
#ifdef HAVE_GETOPT_LONG
		c = getopt_long( argc, args, "tVic:hg:",
				long_options, &option_index);
#else
		c = getopt( argc, args, "tVic:hg:");
#endif
		
		if (c == -1)
			break;
		
		switch (c)
		{
			case 't':
				totrace = true;
				break;
				
			case 'V':
				dprintf( txt_version);
				exit( 0);
				
			case 'i':
				interactive = true;
				break;
				
			case 'c':
				if (config_file)
					free( config_file);
				config_file = xstrdup( optarg);
				break;
				
			case 'h':
				dprintf( txt_version);
				dprintf( txt_help);
				done_machine();
				exit( 0);
				
			case 'g':
				conf_remote_gdb( optarg);
				break;
				
			case '?':
				die( 3, "Unknown parameter or argument required.\n");
				
			default:
				die( 3, "Unknown parameter "
					"'%c'.\n", optopt);
		}
	}
	
	if (optind < argc)
		die( 3, "Unexpected arguments.\n");
}


/****************************************************************************
 * Main function
 */
int
main( int argc, char *args[])

{
	output_init();

	init_machine();
	
	parse_cmdline( argc, args);
	
	script();
	go_machine();
	done_machine();

	return 0;
}
