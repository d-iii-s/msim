/*
 * Simple synchronisation device
 * Copyright (c) 2002-2007 Viliam Holub
 */

/** \file dorder.c
 *
 * Device: Simple synchronisation
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

/** \{ \name  Registers */
#define REGISTER_INT_UP		0	/**< Assert interrupts */
#define REGISTER_INT_PEND	0	/**< Interrupts pending */
#define REGISTER_INT_DOWN 	4	/**< Deassert interrupts */
#define REGISTER_LIMIT		8	/**< Register block size */
/* \} */

/*
 * Device commands
 */

static bool dorder_init( parm_link_s *parm, device_s *dev);
static bool dorder_info( parm_link_s *parm, device_s *dev);
static bool dorder_stat( parm_link_s *parm, device_s *dev);
static bool dorder_synchup( parm_link_s *parm, device_s *dev);
static bool dorder_synchdown( parm_link_s *parm, device_s *dev);

/** Dorder command-line commands and parameters. */
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
	{ "synchup", (cmd_f)dorder_synchup,
		DEFAULT,
		DEFAULT,
		"Write to the synchronization register",
		"Write the synchronization register - enables interrupt pending "
			"on processors with nonzero bits in the mask",
		REQ INT "mask" END},
	{ "synchdown", (cmd_f)dorder_synchdown,
		DEFAULT,
		DEFAULT,
		"Write to the synchronization register",
		"Write the synchronization register - disables interrupt pending "
			"on processors with nonzero bits in the mask",
		REQ INT "mask" END},
	LAST_CMD
};

/** Name of the dorder device as presented to the user */
const char id_dorder[] = "dorder";

static void dorder_done( device_s *d);
static void dorder_read( device_s *d, uint32_t addr, uint32_t *val);
static void dorder_write( device_s *d, uint32_t addr, uint32_t val);

/** Doder object structure */
device_type_s DOrder =
{
	/* Type name and description */
	.name = id_dorder,
	.brief = "Synchronization device",
	.full = "The order device allows to acquire a unique processor number ("
	"a serial number) and assert an interrupt to the specified processor"
	"in the multiprocessor machine.",
	
	/* Functions */
	.done = dorder_done,
	.read = dorder_read,
	.write = dorder_write,
	
	/* Commands */
	dorder_cmds
};


/** Dorder instance data structure. */
struct dorder_data_struct
{
	uint32_t addr;		/**< Dorder address */
	int intno;		/**< Interrupt number */

	long cmds;		/**< Total number of commands */
};


/** Writes to the synchronisation register - generates interrupts.
 *
 * \parm od	Dorder instance data structure
 * \parm val	A value (mask) which identifies processors
 */
static void
sync_up_write( struct dorder_data_struct *od, uint32_t val)
{
	int i;

	od->cmds++;

	for (i=0; i<32; i++,val>>=1)
		if (val & 1)
			dcpu_interrupt_up( i, od->intno);
}


/** Writes to the interrupt-down register - disables pending interrupts.
 *
 * \parm od	Dorder instance data structure
 * \parm val	A value (mask) which identifies processors
 */
static void
sync_down_write( struct dorder_data_struct *od, uint32_t val)
{
	int i;

	od->cmds++;

	for (i=0; i<32; i++,val>>=1)
		if (val & 1)
			dcpu_interrupt_down( i, od->intno);
}



/** Init command implementation.
 *
 * \param parm	Command-line parameters
 * \param dev	Device instance structure
 * \return True if successful
 */
static bool
dorder_init( parm_link_s *parm, device_s *dev)
{
	struct dorder_data_struct *od;
	
	/* Alloc the dorder structure. */
	od = XXMALLOC( struct dorder_data_struct);
	dev->data = od;
	
	/* Initialize */
	parm_next( &parm);
	od->addr = parm_next_int( &parm);
	od->intno = parm_next_int( &parm);
	od->cmds = 0;

	/* Checks */

	/* Address alignment */
	if (!addr_word_aligned( od->addr))
	{
		mprintf( "Dorder address must be 4-byte aligned.\n");
		XFREE( od);
		return false;
	}

	/* Address limit */
	if ((long long)od->addr +(long long)REGISTER_LIMIT > 0x100000000ull)
	{
		mprintf( "Invalid address; registers would exceed the 4GB limit.\n");
		return false;
	}

	/* Interrupt number */
	if (od->intno < 0 || od->intno > 6)
	{
		mprintf( "Interrupt number must be within 0..6.\n");
		XFREE( od);
		return false;
	}
	
	return true;
}


/** Info command implementation
 *
 * \param parm	Command-line parameters
 * \param dev	Device instance structure
 * \return True; always successful
 */
static bool
dorder_info( parm_link_s *parm, device_s *dev)
{
	struct dorder_data_struct *od = dev->data;
	
	info_printf( "address:0x%08x intno:%d\n", od->addr, od->intno);

	return true;
}


/** Stat comamnd implementation.
 *
 * \param parm	Command-line parameters
 * \param dev	Device instance structure
 * \return True; always successful
 */
static bool
dorder_stat( parm_link_s *parm, device_s *dev)
{
	struct dorder_data_struct *od = dev->data;
	
	info_printf( "total cmds count:%ld\n", od->cmds);

	return true;
}


/** Synchup command implementation.
 *
 * \param parm	Command-line parameters
 * \param dev	Device instance structure
 * \return True; always successful
 */
static bool
dorder_synchup( parm_link_s *parm, device_s *dev)
{
	sync_up_write( (struct dorder_data_struct *)dev->data, parm->token.tval.i);
	return true;
}


/** Synchdown command implementation.
 *
 * \param parm	Command-line parameters
 * \param dev	Device instance structure
 * \return True; always successful
 */
static bool
dorder_synchdown( parm_link_s *parm, device_s *dev)
{
	sync_down_write( (struct dorder_data_struct *)dev->data, parm->token.tval.i);
	return true;
}


/*
 *
 * Implicit commands
 */


/** Cleans up the device.
 *
 * \param d	Device instance pointer
 */
static void
dorder_done( device_s *d)
{
	XFREE( d->name);
	XFREE( d->data);
}


/** Read command implementation.
 *
 * \param d	Dorder device pointer
 * \param addr	Address of the read operation
 * \param val	Readed (returned) value
 */
static void
dorder_read( device_s *d, uint32_t addr, uint32_t *val)
{
	struct dorder_data_struct *od = d->data;
	
	if (addr == od->addr +REGISTER_INT_PEND)
		*val = pr->procno;
	else if (addr == od->addr +REGISTER_INT_DOWN)
		*val = 0;
}


/** Write command implementation.
 *
 * \param d	Dorder device pointer
 * \param addr	Written address
 * \param val	Value to write
 */
void
dorder_write( device_s *d, uint32_t addr, uint32_t val)
{
	struct dorder_data_struct *od = d->data;
	
	if (addr == od->addr +REGISTER_INT_UP)
		sync_up_write( od, val);
	else if (addr == od->addr +REGISTER_INT_DOWN)
		sync_down_write( od, val);
}
