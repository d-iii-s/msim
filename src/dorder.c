/*
 * dorder.h
 * Simple synchronisation device
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dorder.h"

#include "device.h"
#include "machine.h"
#include "dcpu.h"
#include "output.h"
#include "parser.h"
#include "utils.h"


/*
 * Device commands
 */

static bool dorder_init( parm_link_s *parm, device_s *dev);
static bool dorder_info( parm_link_s *parm, device_s *dev);
static bool dorder_stat( parm_link_s *parm, device_s *dev);
static bool dorder_synch( parm_link_s *parm, device_s *dev);

cmd_s dorder_cmds[] =
{
	{ "init", (cmd_f)dorder_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "name/order name" NEXT
		REQ INT "addr/order register address" NEXT
		REQ INT "int_no/interrupt number within 0..6" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Displays help",
		"Displays help",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)dorder_info,
		DEFAULT,
		DEFAULT,
		"Displays device state",
		"Displays device state",
		NOCMD},
	{ "stat", (cmd_f)dorder_stat,
		DEFAULT,
		DEFAULT,
		"Displays device statistics",
		"Displays device statistics",
		NOCMD},
	{ "synch", (cmd_f)dorder_synch,
		DEFAULT,
		DEFAULT,
		"Write to the synchronization register",
		"Write the synchronization register - generates interrupts "
			"on processors with nonzero bits in mask",
		REQ INT "mask" END},
	LAST_CMD
};

const char id_dorder[] = "dorder";

static void dorder_done( device_s *d);
static void dorder_read( device_s *d, uint32_t addr, uint32_t *val);
static void dorder_write( device_s *d, uint32_t addr, uint32_t val);

device_type_s DOrder =
{
	/* device type */
	id_dorder,
	
	/* brief description */
	"Synchronization device",
	
	/* full description */
	"The order device allows to acquire a unique processor number (i.e."
	"a serial number) and assert an interrupt to the specified processor"
	"in the multiprocessor machine.",
	
	/* functions */
	dorder_done,	/* done */
	NULL,		/* step */
	dorder_read,	/* read */
	dorder_write,	/* write */
	
	/* commands */
	dorder_cmds
};


struct dorder_data_struct
{
	uint32_t addr;
	int intno;

	uint64_t cmds;
};




static void
swrite( struct dorder_data_struct *od, uint32_t val)

{
	int i;

	if (val & 0x80000000)
		dcpu_interrupt_down( -1, od->intno);
	else
	{
		od->cmds++;

		for (i=0; i<32; i++,val>>=1)
			if (val & 1)
				dcpu_interrupt_up( i, od->intno);
	}
}



/** Init command implementation
 *
 */

static bool
dorder_init( parm_link_s *parm, device_s *dev)

{
	struct dorder_data_struct *od;
	
	/* alloc the dorder structure */
	od = malloc( sizeof( struct dorder_data_struct));
	if (!od)
	{
		mprintf( txt_pub[ 5]);
		return false;
	}
	dev->data = od;
	
	/* initialize */
	parm_next( &parm);
	od->addr = parm_next_int( &parm);
	od->intno = parm_next_int( &parm);
	od->cmds = 0;

	/* checks */
	if (od->addr & 3)
	{
		mprintf( "Dorder address must be 4-byte aligned.\n");
		free( od);
		return false;
	}
	if (od->intno > 6)
	{
		mprintf( "Interrupt number must be within 0..6.\n");
		free( od);
		return false;
	}
	
	return true;
}


/** Info command implementation
 *
 */

static bool
dorder_info( parm_link_s *parm, device_s *dev)

{
	struct dorder_data_struct *od = dev->data;
	
	info_printf( "address:0x%08x intno:%d\n", od->addr, od->intno);

	return true;
}


/** Stat comamnd implementation
 *
 */

static bool
dorder_stat( parm_link_s *parm, device_s *dev)

{
	struct dorder_data_struct *od = dev->data;
	
	info_printf( "total cmds count:%lld\n", od->cmds);

	return true;
}


/** Synch command implementation
 *
 */

static bool
dorder_synch( parm_link_s *parm, device_s *dev)

{
	struct dorder_data_struct *od = dev->data;

	swrite( od, parm->token.tval.i);

	return true;
}


/*
 *
 * Implicit commands
 */


static void
dorder_done( device_s *d)

{
	XFREE( d->name);
	XFREE( d->data);
}


static void
dorder_read( device_s *d, uint32_t addr, uint32_t *val)

{
	struct dorder_data_struct *od = d->data;
	
	if (addr == od->addr)
		*val = pr->procno;
}


void
dorder_write( device_s *d, uint32_t addr, uint32_t val)

{
	struct dorder_data_struct *od = d->data;
	
	if (addr == od->addr)
		swrite( od, val);
}
