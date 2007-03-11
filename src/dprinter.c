/*
 * Simple printer device
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dprinter.h"

#include "mtypes.h"
#include "device.h"
#include "fault.h"
#include "output.h"
#include "utils.h"


/*
 * Device commands
 */

static bool dprinter_init( parm_link_s *parm, device_s *dev);
static bool dprinter_info( parm_link_s *parm, device_s *dev);
static bool dprinter_stat( parm_link_s *parm, device_s *dev);
static bool dprinter_redir( parm_link_s *parm, device_s *dev);
static bool dprinter_stdout( parm_link_s *parm, device_s *dev);

cmd_s printer_cmds[] =
{
	{ "init", (cmd_f)dprinter_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "name/printer name" NEXT
		REQ INT "addr/register address" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Displays this help text",
		"Displays this help text",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)dprinter_info,
		DEFAULT,
		DEFAULT,
		"Displays printer state and configuration",
		"Displays printer state and configuration",
		NOCMD},
	{ "stat", (cmd_f)dprinter_stat,
		DEFAULT,
		DEFAULT,
		"Displays printer statistics",
		"Displays printer statistics",
		NOCMD},
	{ "redir", (cmd_f)dprinter_redir,
		DEFAULT,
		DEFAULT,
		"Redirect output to the specified file",
		"Redirect output to the specified file",
		REQ STR "filename/output file name" END},
	{ "stdout", (cmd_f)dprinter_stdout,
		DEFAULT,
		DEFAULT,
		"Redirect output to the standard output",
		"Redirect output to the standard output",
		NOCMD},
	LAST_CMD
};

const char id_printer[] = "dprinter";

static void printer_done( device_s *d);
static void printer_step( device_s *d);
static void printer_write( device_s *d, uint32_t addr, uint32_t val);

device_type_s DPrinter =
{
	/* device type */
	id_printer,
	
	/* brief description */
	"Printer simulation",
	
	/* full description */
	"Printer device represents a simple character output device. Via "
		"memory-mapped register system can write character to the "
		"specified output like screen, file or another terminal.",
	
	/* functions */
	printer_done,	/* done */
	printer_step,	/* step */
	NULL,		/* read */
	printer_write,	/* write */
	
	/* commands */
	printer_cmds
};


struct printer_data_s
{
	uint32_t addr;
	int step;
	bool flush;
	
	FILE *output_file;
	
	uint64_t count;
};
typedef struct printer_data_s printer_data_s;



/** Init command implementation
 */
static bool
dprinter_init( parm_link_s *parm, device_s *dev)

{
	printer_data_s *pd;
	
	/* the printer structure allocation */
	pd = malloc( sizeof( printer_data_s));
	if (!pd)
	{
		mprintf( txt_pub[ 5]);
		return false;
	}
	dev->data = pd;
	
	/* inicialization */
	parm_next( &parm);
	pd->addr = parm_next_int( &parm);
	pd->step = 0;
	pd->flush = false;
	pd->output_file = stdout;
	
	pd->count = 0;
	
	/* check */
	if (pd->addr & 3)
	{
		mprintf( "Printer address must be in the 4-byte boundary.\n");
		free( pd);
		return false;
	}
	
	return true;
}


/** Redir command implementation
 */
static bool
dprinter_redir( parm_link_s *parm, device_s *dev)

{
	printer_data_s *pd = dev->data;
	const char *const filename = parm_str( parm);
	FILE *new_file;
	
	new_file = fopen( filename, "w");
	if (!new_file)
	{
		io_error( filename);
		mprintf( txt_pub[ 8]);
		
		return false;
	}
	
	if (pd->output_file != stdout)
	{
		if (!fclose( pd->output_file))
		{
			io_error( 0);
			error( txt_pub[ 15]);
			return false;
		}
	}
	
	pd->output_file = new_file;
	return true;
}


/** Stdout command implementation
 */
static bool
dprinter_stdout( parm_link_s *parm, device_s *dev)

{
	printer_data_s *pd = dev->data;

	if (pd->output_file != stdout)
	{
		if (!fclose( pd->output_file))
		{
			io_error( 0);
			return txt_pub[ 15];
		}

		pd->output_file = stdout;
	}
	
	return true;
}


/** Info command implementation
 */
static bool
dprinter_info( parm_link_s *parm, device_s *dev)

{
	printer_data_s *pd = dev->data;
	mprintf_btag( INFO_SPC, "address:0x%08x\n", pd->addr);
	return true;
}


/** Stat command implementation.
 */
static bool
dprinter_stat( parm_link_s *parm, device_s *dev)

{
	printer_data_s *pd = dev->data;
	mprintf_btag( INFO_SPC, "count:%lld\n", pd->count);
	return true;
}




/*
 *
 * Implicit commands
 */

static void
printer_done( device_s *d)

{
	printer_data_s *pd = d->data;

	if (pd->output_file != stdout)
	{
		if (!fclose( pd->output_file))
		{
			io_error( 0);
			error( txt_pub[ 15]);
		}
	}

	XFREE( d->name);
	XFREE( d->data);
}


static void
printer_step( device_s *d)

{
	printer_data_s *pd = d->data;
	
	if (pd->flush && (++pd->step >= 1024))
	{
		pd->step = 0;
		pd->flush = false;
		fflush( pd->output_file);
	}
}


static void
printer_write( device_s *d, uint32_t addr, uint32_t val)

{
	printer_data_s *pd = d->data;

	if (addr == pd->addr) 

	{
		fprintf( pd->output_file, "%c", (char) val);
		pd->flush = true;
		pd->count++;
	}
}
