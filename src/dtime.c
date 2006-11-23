/*
 * Simple host time device
 * Copyright (c) 2003,2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dtime.h"

#include "device.h"
#include "output.h"
#include "utils.h"


/*
 * Device commands
 */

static bool dtime_init( parm_link_s *parm, device_s *dev);
static bool dtime_info( parm_link_s *parm, device_s *dev);
static bool dtime_stat( parm_link_s *parm, device_s *dev);

cmd_s dtime_cmds[] =
{
	{ "init", (cmd_f)dtime_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "Timer name" NEXT
		REQ INT "Timer register address" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Help",
		"Help",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)dtime_info,
		DEFAULT,
		DEFAULT,
		"Configuration information",
		"Configuration information",
		NOCMD},
	{ "stat", (cmd_f)dtime_stat,
		DEFAULT,
		DEFAULT,
		"Statictics",
		"Statictics",
		NOCMD},
	LAST_CMD
};

const char id_dtime[] = "dtime";

static void dtime_done( device_s *d);
static void dtime_read( device_s *d, uint32_t addr, uint32_t *val);

device_type_s DTime =
{
	/* type */
	id_dtime,
	
	/* brief description*/
	"Real time",
	
	/* full description */
	"The time device brings the host real time to the simulated "
		"environment. One memory-mapped register allows programs"
		"to read hosts time since the Epoch as specified in the"
		"POSIX.",
	
	/* functions */
	dtime_done,	/* done */
	NULL,		/* step */
	dtime_read,	/* read */
	NULL,		/* write */
	
	/* commands */
	dtime_cmds
};


struct dtime_data_struct
{
	uint32_t addr;
};


/** Init command implementation
 */
static bool
dtime_init( parm_link_s *parm, device_s *dev)

{
	struct dtime_data_struct *td;
	
	/* alloc the dtime structure */
	td = malloc( sizeof( struct dtime_data_struct));
	if (!td)
	{
		mprintf( txt_pub[ 5]);
		return false;
	}
	dev->data = td;
	
	/* inicialization */
	parm_next( &parm);
	td->addr = parm_next_int( &parm);
	
	/* check */
	if (td->addr & 0x3)
	{
		mprintf( "Dtime address must be 4-byte aligned.\n");
		return false;
	}
	
	return 0;
}


/** Info command implementation
 */
static bool
dtime_info( parm_link_s *parm, device_s *dev)

{
	struct dtime_data_struct *td = dev->data;
	
	mprintf_btag( INFO_SPC, "address:0x%08x", td->addr);
	
	return true;
}


/** Stat command implementation
 */
static bool
dtime_stat( parm_link_s *parm, device_s *dev)

{
	mprintf_btag( INFO_SPC, "no statistics\n");
	return true;
}



/*
 *
 * Implicit commands
 *
 */
static void
dtime_done( device_s *d)

{
	XFREE( d->name);
	XFREE( d->data);
}


static void
dtime_read( device_s *d, uint32_t addr, uint32_t *val)

{
	struct dtime_data_struct *od = d->data;
	
	if (addr == od->addr)
		*val = (uint32_t) time( 0);
}
