/*
 * Copyright (c) 2002-2005 Viliam Holub
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cmd.h"

#include "check.h"
#include "mcons.h"
#include "mtypes.h"
#include "parser.h"
#include "device.h"
#include "machine.h"
#include "debug.h"
#include "text.h"
#include "fault.h"
#include "output.h"
#include "env.h"
#include "utils.h"


#define SETUP_BUF_SIZE		65536


/*
 * System functions
 */
static bool system_add( parm_link_s *pl, void *data);
static bool system_quit( parm_link_s *pl, void *data);
static bool system_md( parm_link_s *pl, void *data);
static bool system_id( parm_link_s *pl, void *data);
static bool system_dd( parm_link_s *pl, void *data);
static bool system_mbd( parm_link_s *pl, void *data);
static bool system_stat( parm_link_s *pl, void *data);
static bool system_echo( parm_link_s *pl, void *data);
static bool system_goto( parm_link_s *pl, void *data);
static bool system_step( parm_link_s *pl, void *data);
static bool system_set( parm_link_s *pl, void *data);
static bool system_unset( parm_link_s *pl, void *data);
static bool system_help( parm_link_s *pl, void *data);

/*
 * TAB completion generator-finders
 */
static void system_add_find_generator( parm_link_s **pl, const cmd_s *cmd,
		gen_f *generator, const void **data);
static void system_set_find_generator( parm_link_s **pl, const cmd_s *cmd,
		gen_f *generator, const void **data);

/*
 * Main command structure
 *
 * All system commands are defined here.
 */
cmd_s system_cmds[] =
{
	{ "init", NULL,	/* hardwired */
		DEFAULT,
		DEFAULT,
		"",
		"",
		NOCMD},
	{ "add", system_add,
		system_add_find_generator,
		DEFAULT,
		"Adds a new device into the system.",
		"Adds a new device into the system.",
		REQ STR "type/Device type" NEXT
		REQ STR "name/Device name which is not in conflict with any "
			"system command or already installed device." CONT},
	{ "rm", NULL,
		DEFAULT,
		DEFAULT,
		"Removes the device from the system.",
		"Removes the specified device from the system.",
		REQ STR "dname/device name" END},
	{ "quit", system_quit,
		DEFAULT,
		DEFAULT,
		"Exits msim.",
		"Exits msim.",
		NOCMD},
	{ "md", system_md,
		DEFAULT,
		DEFAULT,
		"Dumps words from unmapped memory.",
		"Dumps words from unmapped memory.",
		REQ INT "addr/memory address" NEXT
		REQ INT "cnt/count" END},
	{ "id", system_id,
		DEFAULT,
		DEFAULT,
		"Dumps instructions from unmapped memory.",
		"Dumps instructions from unmapped memory.",
		REQ INT "addr/memory address" NEXT
		REQ INT "cnt/count" END},
	{ "dd", system_dd,
		DEFAULT,
		DEFAULT,
		"Dumps all installed devices.",
		"Dumps all installed devices.",
		NOCMD},
	{ "mbd", system_mbd,
		DEFAULT,
		DEFAULT,
		"Dumps all installed memory blocks.",
		"Dumps all installed memory blocks.",
		NOCMD},
	{ "stat", system_stat,
		DEFAULT,
		DEFAULT,
		"Dumps available statistic information.",
		"Dumps available statistic information.",
		NOCMD},
	{ "echo", system_echo,
		DEFAULT,
		DEFAULT,
		"Prints user message.",
		"Prints user message.",
		OPT STR "text" END},
	{ "goto", system_goto,
		DEFAULT,
		DEFAULT,
		"Go to the specified address or continue.",
		"Go to the specified address or continue.",
		OPT STR "addr" END},
	{ "step/s", system_step,
		DEFAULT,
		DEFAULT,
		"Simulates one or a specified number of instructions.",
		"Simulates one or a specified number of instructions.",
		OPT INT "cnt/intruction count" END},
	{ "set", system_set,
		system_set_find_generator,
		DEFAULT,
		"Sets enviroment variables.",
		"Sets configuration variables.",
		OPT STR "name/variable name" NEXT
		OPT CON "=" NEXT
		REQ VAR "val/value" END},
	{ "unset", system_unset,
		DEFAULT,
		DEFAULT,
		"Unsets enviroment variables.",
		"Unsets environment variables.",
		REQ STR "name/variable name" END},
	{ "help", system_help,
		DEFAULT,
		DEFAULT,
		"Displays a help text.",
		"Displays a help text.",
		OPT STR "cmd/command name" END},

	LAST_CMD
};


/* config error */
static void
conf_error( const char *msg, int lineno)

{
	if (lineno == -1)
		error( "error: %s", msg);
	else
		error( "error(%d): %s", lineno, msg);
}


/** Searchies for device type and allocs device structure.
 */
static device_s *
alloc_device( const char *dtype, const char *dname)

{
	device_s *d;
	const device_type_s **dt;

	/* search for device type */
	for (dt = device_types; *dt; dt++)
		if (!strcmp( dtype, (*dt)->name))
			break;

	if (!*dt)
	{
		conf_error( "Unknown device type", -1);
		return NULL;
	}

	/* alloc a new instance */
	d = (device_s *)malloc( sizeof( *d));
	if (!d)
	{
		conf_error( "Not enough memory", -1);
		return NULL;
	}

	/* inicialization */
	d->type = *dt;
	d->name = xstrdup( dname);
	d->data = NULL;
	d->next = NULL;

	if (!d->name)
	{
		free( d);
		conf_error( "Not enough memory", -1);
		return NULL;
	}
	
	return d;
}


/** Add command implementation.
 * 
 * adding memory, devices, etc.
 * syntax:
 * add what name parameters...
 * 
 * Device name should not be same as command name and there should not be
 * another device with a same name.
 */
static bool
system_add( parm_link_s *pl, void *data)

{
	device_s *d;

	/* check for conflicts between the device name and a command name */
	if (cmd_find( parm_str( pl), system_cmds, NULL) == CMP_HIT)
	{
		dprintf( "Device name '%s' is in conflict with a command name.\n",
				parm_str( pl));
		return false;
	}
	if (dev_by_name( parm_str( pl)))
	{
		dprintf( "Device name duplicity\n");
		return false;
	}
	
	/* alloc device */
	d = alloc_device( pl->token.tval.s, pl->next->token.tval.s);
	if (!d)
		return false;
	
	/* call device inicialization */
	if (!cmd_run_by_name( "init", pl->next, d->type->cmds, d))
	{
		free( d->name);
		free( d);
		return false;
	}

	/* add into the device list */
	dev_add( d);
	
	return true;
}


/*
 * the goto command implementation
 */
static bool
system_goto( parm_link_s *pl, void *data)

{
	if (pl->token.ttype == tt_end)
		interactive = false;
	else
	{
		breakpoint = true;
		breakpointaddr = pl->token.tval.i & ~0x3;
	}

	return true;
}


/*
 * system_step
 * executes a given count of instructions
 */
static bool
system_step( parm_link_s *pl, void *data)

{
	switch (pl->token.ttype)
	{
		case tt_end:
			stepping = 1;
			interactive = false;
			break;
		case tt_int:
			stepping = pl->token.tval.i;
			interactive = false;
			break;
		default:
			break;
	}

	return true;
}


/** Set command implementation
 *
 */
static bool
system_set( parm_link_s *pl, void *data)

{
	return env_cmd_set( pl);
}


/** Unset command implementation
 *
 */
static bool
system_unset( parm_link_s *pl, void *data)

{
	return env_cmd_unset( pl);
}



/* Instruction dump */
static bool
system_id( parm_link_s *pl, void *data)

{
	int i;
	TInstrInfo ii;
	uint32_t p1, p2;
	
	p1 = pl->token.tval.i & ~0x3;
	p2 = pl->next->token.tval.i;
	
	for (i=0; i<p2; i++, p1+=4)
	{
			
		ii.icode = mem_read( p1);
		decode_instr( &ii);
		iview( p1, &ii, true, 0);
	}
	
	return true;
}


/* device dump */
static bool
system_dd( parm_link_s *pl, void *data)

{
	dbg_dev_dump();
	return true;
}


/* memory block dump */
static bool
system_mbd( parm_link_s *pl, void *data)

{
	dbg_msd_dump();
	return true;
}


/* statistic dump */
static bool
system_stat( parm_link_s *pl, void *data)

{
	dbg_dev_stat();
	return true;
}


/* memory dump */
static bool
system_md( parm_link_s *pl, void *data)

{
	unsigned int j;
	uint32_t val, p1, p2;
	
	p1 = pl->token.tval.i & ~0x3;
	p2 = pl->next->token.tval.i;

	for (j=0; j<p2; j++, p1+=4)
	{
		if (!(j&0x3))
			dprintf( "  %08x    ", p1);
		
		val = mem_read( p1);
		
		dprintf( "%08x  ", val);
	
		if ((j&0x3) == 3)
			dprintf( "\n");
	}
	
	if (j)
		dprintf( "\n");
	
	return true;
}



/** quit command
 * 
 * Quits msim immediately.
 */
static bool
system_quit( parm_link_s *pl, void *data)

{
	interactive = false;
	tohalt = true;

	return true;
}


/** echo command
 * 
 * Prints the user test on the screen.
 */
static bool
system_echo( parm_link_s *pl, void *data)

{
	dprintf( "%s\n", (pl->token.ttype==tt_str) ? pl->token.tval.s : "\n");
	return true;
}


/** help command
 * 
 * Prints the help.
 */
static bool
system_help( parm_link_s *pl, void *data)

{
	cmd_print_extended_help( pl, system_cmds);
	return true;
}


/* Interprets the command line.
 *
 * Line is terminated by '\0' or '\n'.
 */
bool
interpret( const char *s)

{
	bool b;
	parm_link_s *pl;
	device_s *dev;

	/* parse input */
	pl = parm_parse( s);
	if (!pl)
	{
		conf_error( "Not enough memory.", lineno);
		return false;
	}

	if (pl->token.ttype == tt_end)
		return true;

	if (pl->token.ttype != tt_str)
	{
		dprintf( "Command name expected.\n");
		return true;
	}

	/* run command */
	if ((dev = dev_by_name( pl->token.tval.s)))
		/* device command */
		b = cmd_run_by_parm( pl->next, dev->type->cmds, dev);
	else
		/* system command */
		b = cmd_run_by_parm( pl, system_cmds, NULL);
	
	parm_delete( pl);

	return b;
}


/**  setup_apply - Runs initial script uploaded in the memory.
 */
static bool
setup_apply( const char *buf)

{
	lineno = 1;
	
	while ((*buf) && !tohalt)
	{
		if (!interpret( buf) && halt_on_error)
				die( ERR_INIT, 0);

		lineno++;
		
		/* move to the next line */
		while (*buf && (*buf != '\n'))
			buf++;
		if (buf)
			buf++;
	}

	lineno = -1;
	
	return true;
}


/* interprets configuration file */
void
script()

{
	int fd, readed;
	char *buf;
	
	buf = (char *) malloc( SETUP_BUF_SIZE);
	if (!buf)
		die( ERR_MEM, "Not enough memory.");
	
	if (!config_file)
	{
		/* checking for variable MSIMCONF */
		config_file = getenv( "MSIMCONF");
		if (!config_file)
			config_file = "msim.conf";
	}
	
	/* opening configuration file */
	fd = open( config_file, O_RDONLY);
	if (fd == -1)
	{
		if (errno != ENOENT)
			io_die( ERR_IO, config_file);
		
		interactive = true;
	}
	else
	{
		/* reading configuration file */
		readed = read( fd, buf, SETUP_BUF_SIZE);
		if (readed == -1)
		{
			io_error( config_file);
			if (close( fd))
				io_error( config_file);
			
			die( ERR_IO, 0);
		}
		
		if (readed > SETUP_BUF_SIZE)
			readed = SETUP_BUF_SIZE;
		buf[ readed] = 0;
		
		if (close( fd))
			io_die( ERR_IO, config_file);
		
		script_stat = true;
		setup_apply( buf);
		script_stat = false;
	}
 	
	free( buf);
}



/** generator_devtype - Generates a list of device types.
 */
char *
generator_devtype( parm_link_s *pl, const void *data, int level)

{
	const char *s;
	const static device_type_s **d;

	RQ( pl != NULL);
	RQ( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	if (level == 0)
		d = NULL;
	
	if (parm_type( pl) == tt_str)
		s = dev_by_partial_typename( parm_str( pl), &d);
	else
		s = dev_by_partial_typename( "", &d);
	
	return s ? xstrdup( s) : NULL;
}



/** generator_devname - Generates a list of installed device names.
 */
char *
generator_devname( parm_link_s *pl, const void *data, int level)

{
	const char *s;
	static device_s *d;

	RQ( pl != NULL);
	RQ( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	if (level == 0)
		d = NULL;
	
	if (parm_type( pl) == tt_str)
		s = dev_by_partial_name( parm_str( pl), &d);
	else
		s = dev_by_partial_name( "", &d);
	
	return s ? xstrdup( s) : NULL;
}


/** generator_system - Generates a list of commands and device names.
 */
char *
generator_system( parm_link_s *pl, const void *data, int level)

{
	const char *s;
	static enum {command_name, device_name} gen_type;

	if (level == 0)
		gen_type = command_name;

	if (gen_type == command_name)
	{
		s = generator_cmd( pl, system_cmds+1, level);
		if (!s)
		{
			gen_type = device_name;
			level = 0;
		}
	}

	if (gen_type == device_name)
		s = generator_devname( pl, NULL, level);

	return s ? xstrdup( s) : NULL;
}


/** system_add_find_generator
 */
static void
system_add_find_generator( parm_link_s **pl, const cmd_s *cmd,
		gen_f *generator, const void **data)

{
	RQ( pl != NULL, data != NULL, generator != NULL, cmd != NULL);
	RQ( *generator == NULL, *pl != NULL, *data == NULL);

	if (parm_type( *pl) == tt_str
			&& dev_by_partial_typename( parm_str( *pl), NULL)
			&& parm_type( (*pl)->next) == tt_end)
		*generator = generator_devtype; 
}


/** system_set_find_generator
 */
static void
system_set_find_generator( parm_link_s **pl, const cmd_s *cmd,
		gen_f *generator, const void **data)

{
	int res;

	RQ( pl != NULL, data != NULL, generator != NULL, cmd != NULL);
	RQ( *generator == NULL, *pl != NULL, *data == NULL);

	if (parm_type( *pl) == tt_str)
	{
		// look up for variable name
		res = env_cnt_partial_varname( parm_str( *pl));
		if (res == 0)
			return; // error
		if (res == 1)
		{
			// variable fit by partial name
			if (parm_type( (*pl)->next) == tt_end)
					*generator = generator_envname;
			else
			{
				// continue
				if (env_check_varname( parm_str( *pl)))
				{
					parm_next( pl);
					if (parm_type( *pl) == tt_str)
					{
						if (!strcmp( parm_str( *pl), "="))
						{
							// search for value
							// XXX
						}
						else if (!parm_str( *pl)[ 0])
							*generator = generator_equal_char;
						else
							; // error
					}
					else
						; // error
				}
				else
					; // error
			}
		}
		else
		{
			// multiple hit
			if (parm_type( (*pl)->next) == tt_end)
				*generator = generator_envname; 
			else
				; // error
		}
	}
}


/** find_system_generator - Looks up for the completion generator.
 * 
 * The command is specified by the first parameter.
 */
void
find_system_generator( parm_link_s **pl,
		gen_f *generator, const void **data)

{
	int res;
	const cmd_s *cmd;
	parm_link_s *plx;

	RQ( pl != NULL, generator != NULL, data != NULL);
	RQ( *pl != NULL, *generator == NULL, *data == NULL);

	if (parm_type( *pl) == tt_end)
	{
		*generator = generator_system;
		return;
	}

	/* check if the first token is a string */
	if (parm_type( *pl) != tt_str)
		return;

	/* find fine command */
	res = cmd_find( parm_str( *pl), system_cmds+1, &cmd);
	switch (res)
	{
		case CMP_NH:
			// unknown command - check device names
			{
				int cnt;
				device_s *d = NULL;

				cnt = devs_by_partial_name( parm_str( *pl), &d);
				if (cnt == 0)
					break;
				else
				{
					plx = *pl;
					parm_next( pl);
					if (parm_type( *pl) == tt_end)
					{
						*generator = generator_system;
						*pl = plx;
						break;
					}
					else
					{
						if (cnt > 1)
							break;

						find_dev_gen( pl, d, generator, data);
					}
				}
			}
			break;

		case CMP_MHIT:
		case CMP_HIT:
			plx = *pl;
			parm_next( pl);
			if (parm_type( *pl) == tt_end)
			{
				// set the default system generator
				*generator = generator_system; 
				*pl = plx;
				break;
			}
			else
			{
				if (res == CMP_MHIT)
					// input error
					break;

				// continue to the next generator, if possible
				if (cmd->find_gen)
					cmd->find_gen( pl, cmd, generator,
							data);
			}
			break;
	}
}
