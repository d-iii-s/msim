/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Reading and executing commands
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "main.h"
#include "cmd.h"
#include "check.h"
#include "mcons.h"
#include "device/device.h"
#include "device/machine.h"
#include "debug/debug.h"
#include "fault.h"
#include "output.h"
#include "utils.h"
#include "env.h"
#include "cline.h"


/**< System functions */
static bool system_add(parm_link_s *pl, void *data);
static bool system_quit(parm_link_s *pl, void *data);
static bool system_md(parm_link_s *pl, void *data);
static bool system_id(parm_link_s *pl, void *data);
static bool system_dd(parm_link_s *pl, void *data);
static bool system_mbd(parm_link_s *pl, void *data);
static bool system_stat(parm_link_s *pl, void *data);
static bool system_echo(parm_link_s *pl, void *data);
static bool system_continue(parm_link_s *pl, void *data);
static bool system_step(parm_link_s *pl, void *data);
static bool system_set(parm_link_s *pl, void *data);
static bool system_unset(parm_link_s *pl, void *data);
static bool system_help(parm_link_s *pl, void *data);


/**< TAB completion generator-finders */
static void system_add_find_generator(parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data);
static void system_set_find_generator(parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data);
static void system_unset_find_generator(parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data);


/**< Main command structure
 *
 * All system commands are defined here.
 *
 */
static cmd_s system_cmds[] = {
	{
		"init",
		NULL,    /* hardwired */
		DEFAULT,
		DEFAULT,
		"",
		"",
		NOCMD
	},
	{
		"add",
		system_add,
		system_add_find_generator,
		DEFAULT,
		"Add a new device into the system.",
		"Add a new device into the system.",
		REQ STR "type/Device type" NEXT
		REQ STR "name/Device name" CONT
	},
	{
		"quit",
		system_quit,
		DEFAULT,
		DEFAULT,
		"Exit msim.",
		"Exit msim.",
		NOCMD
	},
	{
		"md",
		system_md,
		DEFAULT,
		DEFAULT,
		"Dump words from unmapped memory.",
		"Dump words from unmapped memory.",
		REQ INT "addr/memory address" NEXT
		REQ INT "cnt/count" END
	},
	{
		"id",
		system_id,
		DEFAULT,
		DEFAULT,
		"Dump instructions from unmapped memory.",
		"Dump instructions from unmapped memory.",
		REQ INT "addr/memory address" NEXT
		REQ INT "cnt/count" END
	},
	{
		"dd",
		system_dd,
		DEFAULT,
		DEFAULT,
		"Dump all installed devices.",
		"Dump all installed devices.",
		NOCMD
	},
	{
		"mbd",
		system_mbd,
		DEFAULT,
		DEFAULT,
		"Dump all installed memory blocks.",
		"Dump all installed memory blocks.",
		NOCMD
	},
	{
		"stat",
		system_stat,
		DEFAULT,
		DEFAULT,
		"Dump available statistic information.",
		"Dump available statistic information.",
		NOCMD
	},
	{
		"echo",
		system_echo,
		DEFAULT,
		DEFAULT,
		"Print user message.",
		"Print user message.",
		OPT STR "text" END
	},
	{
		"continue",
		system_continue,
		DEFAULT,
		DEFAULT,
		"Continue simulation.",
		"Continue simulation.",
		NOCMD
	},
	{
		"step/s",
		system_step,
		DEFAULT,
		DEFAULT,
		"Simulate one or a specified number of instructions.",
		"Simulate one or a specified number of instructions.",
		OPT INT "cnt/intruction count" END
	},
	{
		"set",
		system_set,
		system_set_find_generator,
		DEFAULT,
		"Set enviroment variables.",
		"Set configuration variables.",
		OPT STR "name/variable name" NEXT
		OPT CON "=" NEXT
		REQ VAR "val/value" END
	},
	{
		"unset",
		system_unset,
		system_unset_find_generator,
		DEFAULT,
		"Unset enviroment variables.",
		"Unset environment variables.",
		REQ STR "name/variable name" END
	},
	{
		"help",
		system_help,
		DEFAULT,
		DEFAULT,
		"Display a help text.",
		"Display a help text.",
		OPT STR "cmd/command name" END
	},
	LAST_CMD
};


/** Searches for device type and allocates device structure
 *
 */
static device_s *alloc_device(const char *dtype, const char *dname)
{
	device_s *dev;
	const device_type_s **type;

	/* Search for device type */
	for (type = device_types; *type; type++)
		if (!strcmp(dtype, (*type)->name))
			break;

	if (!*type) {
		intr_error("Unknown device type");
		return NULL;
	}

	/* Allocate a new instance */
	dev = (device_s *) malloc(sizeof(*dev));
	if (!dev) {
		intr_error("Not enough memory to allocate device");
		return NULL;
	}

	/* Inicialization */
	dev->type = *type;
	dev->name = xstrdup(dname);
	dev->data = NULL;
	dev->next = NULL;

	if (!dev->name) {
		free(dev);
		intr_error("Not enough memory to allocate device name");
		return NULL;
	}
	
	return dev;
}


/** Add command implementation
 * 
 * Add memory, devices, etc.
 * 
 * Device name should not be the same as
 * a command name and there should not be
 * another device with a same name.
 *
 */
static bool system_add(parm_link_s *pl, void *data)
{
	device_s *dev;

	/* Check for conflicts between
	 * the device name and a command name */
	if (cmd_find(parm_str(pl), system_cmds, NULL) == CMP_HIT) {
		mprintf("Device name '%s' is in conflict with a command name\n",
				parm_str(pl));
		return false;
	}
	
	if (dev_by_name(parm_str( pl))) {
		mprintf("Device name duplicity\n");
		return false;
	}
	
	/* Allocate device */
	dev = alloc_device(pl->token.tval.s, pl->next->token.tval.s);
	if (!dev)
		return false;
	
	/* Call device inicialization */
	if (!cmd_run_by_name("init", pl->next, dev->type->cmds, dev)) {
		free(dev->name);
		free(dev);
		return false;
	}

	/* Add into the device list */
	dev_add(dev);
	
	return true;
}


/** Continue command implementation
 *
 * Continue simulation.
 *
 */
static bool system_continue(parm_link_s *pl, void *data)
{
	interactive = false;
	return true;
}


/* Step command implementation
 *
 * Execute a given count of instructions.
 *
 */
static bool system_step(parm_link_s *pl, void *data)
{
	switch (pl->token.ttype) {
	case tt_end:
		stepping = 1;
		interactive = false;
		break;
	case tt_int:
		stepping = pl->token.tval.i;
		interactive = false;
		break;
	default:
		return false;
	}

	return true;
}


/** Set command implementation
 *
 * Set configuration variable.
 *
 */
static bool system_set(parm_link_s *pl, void *data)
{
	return env_cmd_set(pl);
}


/** Unset command implementation
 *
 * Unset configuration variable.
 *
 */
static bool system_unset(parm_link_s *pl, void *data)
{
	return env_cmd_unset(pl);
}


/** Dump command implementation
 *
 * Dump instructions.
 *
 */
static bool system_id(parm_link_s *pl, void *data)
{
	uint32_t addr = ALIGN_DOWN(pl->token.tval.i, 4);
	unsigned int cnt = pl->next->token.tval.i;
	
	for (; cnt > 0; addr += 4, cnt--) {
		instr_info ii;
		ii.icode = mem_read(addr);
		decode_instr(&ii);
		iview(addr, &ii, false, 0);
	}
	
	return true;
}


/** Dd command implementation
 *
 * Dump configured devices.
 *
 */
static bool system_dd(parm_link_s *pl, void *data)
{
	dbg_dev_dump();
	return true;
}


/** Mbd command implementation
 *
 * Dump configured memory blocks.
 *
 */
static bool system_mbd(parm_link_s *pl, void *data)
{
	dbg_msd_dump();
	return true;
}


/** Stat command implementation
 *
 * Print simulator statistics.
 *
 */
static bool system_stat(parm_link_s *pl, void *data)
{
	dbg_dev_stat();
	return true;
}


/** Md command implementation
 *
 * Dump memory.
 *
 */
static bool system_md(parm_link_s *pl, void *data)
{
	uint32_t addr = ALIGN_DOWN(pl->token.tval.i, 4);
	unsigned int cnt = pl->next->token.tval.i;
	unsigned int i;
	
	for (i = 0; i < cnt; addr += 4, i++) {
		if ((i & 0x3) == 0)
			mprintf("  %08x    ", addr);
		
		uint32_t val = mem_read(addr);
		mprintf("%08x  ", val);
		
		if ((i & 0x3) == 3)
			mprintf("\n");
	}
	
	if (i != 0)
		mprintf("\n");
	
	return true;
}


/** Quit command implementation
 * 
 * Quit msim immediately.
 *
 */
static bool system_quit(parm_link_s *pl, void *data)
{
	interactive = false;
	tohalt = true;

	return true;
}


/** Echo command implementation
 * 
 * Print the user text on the screen.
 *
 */
static bool system_echo(parm_link_s *pl, void *data)
{
	mprintf("%s\n", (pl->token.ttype == tt_str) ? pl->token.tval.s : "\n");
	return true;
}


/** Help command implementation
 * 
 * Print the help.
 *
 */
static bool system_help(parm_link_s *pl, void *data)
{
	cmd_print_extended_help(pl, system_cmds);
	return true;
}


/** Interprets the command line.
 *
 * Line is terminated by '\0' or '\n'.
 *
 */
bool interpret(const char *str)
{
	/* Parse input */
	parm_link_s *pl = parm_parse(str);
	if (!pl) {
		intr_error("Not enough memory to parse command.");
		return false;
	}
	
	if (pl->token.ttype == tt_end)
		return true;
	
	if (pl->token.ttype != tt_str) {
		mprintf("Command name expected.\n");
		return true;
	}

	bool ret;
	device_s *dev;
	
	/* Run command */
	if ((dev = dev_by_name(pl->token.tval.s)))
		/* Device command */
		ret = cmd_run_by_parm(pl->next, dev->type->cmds, dev);
	else
		/* System command */
		ret = cmd_run_by_parm(pl, system_cmds, NULL);
	
	parm_delete(pl);

	return ret;
}


/** Run initial script uploaded in the memory
 *
 */
static bool setup_apply(const char *buf)
{
	lineno = 1;
	
	while ((*buf) && (!tohalt)) {
		if (!interpret(buf) && (script_stage))
			die(ERR_INIT, 0);
		
		lineno++;
		
		/* Move to the next line */
		while ((*buf) && (*buf != '\n'))
			buf++;
		
		if (buf)
			buf++;
	}
	
	return true;
}


/** Interpret configuration file
 *
 */
void script(void)
{
	char *buf;
	
	buf = (char *) malloc(SETUP_BUF_SIZE);
	if (!buf)
		die(ERR_MEM, "Not enough memory for buffer.");
	
	if (!config_file) {
		/* Checking for variable MSIMCONF */
		config_file = getenv("MSIMCONF");
		if (!config_file)
			config_file = "msim.conf";
	}
	
	/* Opening configuration file */
	int fd = open(config_file, O_RDONLY);
	if (fd == -1) {
		if (errno != ENOENT)
			io_die(ERR_IO, config_file);
		
		interactive = true;
	} else {
		/* Reading configuration file */
		ssize_t rd = read(fd, buf, SETUP_BUF_SIZE);
		if (rd == -1) {
			io_error(config_file);
			if (close(fd))
				io_error(config_file);
			
			die(ERR_IO, 0);
		}
		
		if (rd > SETUP_BUF_SIZE)
			rd = SETUP_BUF_SIZE;
		buf[rd] = 0;
		
		if (close(fd))
			io_die(ERR_IO, config_file);
		
		set_script_stage(config_file);
		setup_apply(buf);
		unset_script_stage();
	}
 	
	free(buf);
}


/** Generate a list of device types
 *
 */
static char *generator_devtype(parm_link_s *pl, const void *data, int level)
{
	const char *str;
	const static device_type_s **type;

	PRE(pl != NULL);
	PRE((parm_type(pl) == tt_str) || (parm_type(pl) == tt_end));
	
	if (level == 0)
		type = NULL;
	
	if (parm_type(pl) == tt_str)
		str = dev_by_partial_typename(parm_str(pl), &type);
	else
		str = dev_by_partial_typename("", &type);
	
	return str ? xstrdup(str) : NULL;
}


/** Generate a list of installed device names
 *
 */
static char *generator_devname(parm_link_s *pl, const void *data, int level)
{
	const char *str;
	static device_s *dev;

	PRE(pl != NULL);
	PRE((parm_type(pl) == tt_str) || (parm_type(pl) == tt_end));
	
	if (level == 0)
		dev = NULL;
	
	if (parm_type(pl) == tt_str)
		str = dev_by_partial_name(parm_str(pl), &dev);
	else
		str = dev_by_partial_name("", &dev);
	
	return str ? xstrdup(str) : NULL;
}


/** Generate a list of commands and device names
 *
 */
static char *generator_system(parm_link_s *pl, const void *data, int level)
{
	const char *str = NULL;
	static enum {
		command_name,
		device_name
	} gen_type;

	if (level == 0)
		gen_type = command_name;

	if (gen_type == command_name) {
		str = generator_cmd(pl, system_cmds + 1, level);
		if (!str) {
			gen_type = device_name;
			level = 0;
		}
	}

	if (gen_type == device_name)
		str = generator_devname(pl, NULL, level);

	return str ? xstrdup(str) : NULL;
}


/** Add command find generator
 *
 */
static void system_add_find_generator(parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data)
{
	PRE(pl != NULL, data != NULL, generator != NULL, cmd != NULL);
	PRE(*generator == NULL, *pl != NULL, *data == NULL);
	
	if ((parm_type(*pl) == tt_str)
		&& (dev_by_partial_typename(parm_str(*pl), NULL))
		&& (parm_type((*pl)->next) == tt_end))
		*generator = generator_devtype; 
}


/** Set command find generator
 *
 */
static void system_set_find_generator( parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data)
{
	PRE(pl != NULL, data != NULL, generator != NULL, cmd != NULL);
	PRE(*generator == NULL, *pl != NULL, *data == NULL);

	if (parm_type( *pl) == tt_str) {
		int res;
		
		/* Look up for a variable name */
		if (parm_type((*pl)->next) == tt_end)
			/* There is a completion possible */
			res = env_cnt_partial_varname(parm_str( *pl));
		else
			/* Exactly one match is allowed */
			res = 1;

		if (res == 0)
			/* Error */
			return;
		
		if (res == 1) {
			/* Variable fit by partial name */
			if (parm_type((*pl)->next) == tt_end)
				*generator = generator_env_name;
			else {
				var_type_e type;

				if (env_check_varname(parm_str( *pl), &type)) {
					parm_next(pl);
					if (parm_type(*pl) == tt_str) {
						if (!strcmp(parm_str(*pl), "=")) {
							/* Search for value */
							parm_next(pl);

							if ((parm_type(*pl) == tt_str) && (type == vt_bool)) {
								if (parm_type((*pl)->next) == tt_end)
									*generator = generator_env_booltype;
							}
						} else if (!parm_str(*pl)[0])
							*generator = generator_equal_char;
					}
				}
			}
		} else  {
			/* Multiple hit */
			if (parm_type((*pl)->next) == tt_end)
				*generator = generator_env_name; 
		}
	}
}


/** Unset command find generator
 *
 */
static void system_unset_find_generator(parm_link_s **pl, const cmd_s *cmd,
	gen_f *generator, const void **data)
{
	PRE(pl != NULL, data != NULL, generator != NULL, cmd != NULL);
	PRE(*generator == NULL, *pl != NULL, *data == NULL);

	if (parm_type(*pl) == tt_str) {
		/* Look up for a variable name */
		int res = env_cnt_partial_varname(parm_str(*pl));

		if (res == 0)
			/* Error */
			return;
		else {
			/* Partially fit by partial name */
			if (parm_type((*pl)->next) == tt_end)
				*generator = generator_bool_envname;
		}
	}
}


/** Look up for the completion generator
 * 
 * The command is specified by the first parameter.
 *
 */
void find_system_generator(parm_link_s **pl, gen_f *generator,
	const void **data)
{
	parm_link_s *orig_pl;
	int cnt;
	device_s *dev;
	const cmd_s *cmd;
	
	PRE(pl != NULL, generator != NULL, data != NULL);
	PRE(*pl != NULL, *generator == NULL, *data == NULL);

	if (parm_type(*pl) == tt_end) {
		*generator = generator_system;
		return;
	}

	/* Check if the first token is a string */
	if (parm_type(*pl) != tt_str)
		return;

	/* Find a command */
	int res = cmd_find(parm_str(*pl), system_cmds + 1, &cmd);
	switch (res) {
	case CMP_NH: /* Unknown command - check device names */
		dev = NULL;
		cnt = devs_by_partial_name(parm_str(*pl), &dev);
		
		if (cnt == 0)
			break;
		else {
			orig_pl = *pl;
			parm_next(pl);
			if (parm_type(*pl) == tt_end) {
				*generator = generator_system;
				*pl = orig_pl;
				break;
			} else {
				if (cnt > 1)
					break;
				
				find_dev_gen(pl, dev, generator, data);
			}
		}
		break;
	case CMP_MHIT:
	case CMP_HIT:
		orig_pl = *pl;
		parm_next(pl);
		if (parm_type(*pl) == tt_end) {
			/* Set the default system generator */
			*generator = generator_system; 
			*pl = orig_pl;
			break;
		} else {
			if (res == CMP_MHIT)
				/* Input error */
				break;

			/* continue to the next generator,
			   if possible */
			if (cmd->find_gen)
				cmd->find_gen(pl, cmd, generator, data);
		}
		break;
	}
}
