/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Synchronization device
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "dorder.h"
#include "device.h"
#include "machine.h"
#include "dcpu.h"
#include "../io/output.h"
#include "../parser.h"
#include "../utils.h"

/** \{ \name Registers */
#define REGISTER_INT_UP   0  /**< Assert interrupts */
#define REGISTER_INT_PEND 0  /**< Interrupts pending */
#define REGISTER_INT_DOWN 4  /**< Deassert interrupts */
#define REGISTER_LIMIT    8  /**< Register block size */
/* \} */

/*
 * Device commands
 */

static bool dorder_init(token_t *parm, device_t *dev);
static bool dorder_info(token_t *parm, device_t *dev);
static bool dorder_stat(token_t *parm, device_t *dev);
static bool dorder_synchup(token_t *parm, device_t *dev);
static bool dorder_synchdown(token_t *parm, device_t *dev);

/** Dorder command-line commands and parameters */
cmd_t dorder_cmds[] = {
	{
		"init",
		(fcmd_t) dorder_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/order name" NEXT
		REQ INT "addr/order register address" NEXT
		REQ INT "int_no/interrupt number within 0..6" END
	},
	{
		"help",
		(fcmd_t) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display help",
		"Display help",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(fcmd_t) dorder_info,
		DEFAULT,
		DEFAULT,
		"Display device state",
		"Display device state",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dorder_stat,
		DEFAULT,
		DEFAULT,
		"Display device statistics",
		"Display device statistics",
		NOCMD
	},
	{
		"synchup",
		(fcmd_t) dorder_synchup,
		DEFAULT,
		DEFAULT,
		"Write to the synchronization register",
		"Write the synchronization register - enables interrupt pending "
			"on processors with nonzero bits in the mask",
		REQ INT "mask" END
	},
	{
		"synchdown",
		(fcmd_t) dorder_synchdown,
		DEFAULT,
		DEFAULT,
		"Write to the synchronization register",
		"Write the synchronization register - disables interrupt pending "
			"on processors with nonzero bits in the mask",
		REQ INT "mask" END
	},
	LAST_CMD
};

/** Name of the dorder device as presented to the user */
const char id_dorder[] = "dorder";

static void dorder_done(device_t *dev);
static void dorder_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val);
static void dorder_write(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t val);

/** Doder object structure */
device_type_s dorder = {
	/* Type name and description */
	.name = id_dorder,
	.brief = "Synchronization device",
	.full =
	    "The order device allows to acquire a unique processor number "
	    "(a serial number) and assert an interrupt to the specified "
	    "processor in the multiprocessor machine.",
	
	/* Functions */
	.done = dorder_done,
	.read = dorder_read,
	.write = dorder_write,
	
	/* Commands */
	dorder_cmds
};

/** Dorder instance data structure */
typedef struct {
	uint32_t addr;  /**< Dorder address */
	int intno;      /**< Interrupt number */
	
	uint64_t cmds;  /**< Total number of commands */
} dorder_data_s;

/** Write to the synchronisation register - generate interrupts.
 *
 * @param od Dorder instance data structure
 * @param val A value (mask) which identifies processors
 *
 */
static void sync_up_write(dorder_data_s *od, uint32_t val)
{
	unsigned int i;
	od->cmds++;
	
	for (i = 0; i < 32; i++, val >>= 1)
		if (val & 1)
			dcpu_interrupt_up(i, od->intno);
}

/** Write to the interrupt-down register - disable pending interrupts.
 *
 * @param od  Dorder instance data structure
 * @param val A value (mask) which identifies processors
 *
 */
static void sync_down_write(dorder_data_s *od, uint32_t val)
{
	unsigned int i;
	
	od->cmds++;
	
	for (i = 0; i < 32; i++, val >>= 1)
		if (val & 1)
			dcpu_interrupt_down(i, od->intno);
}

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true if successful
 *
 */
static bool dorder_init(token_t *parm, device_t *dev)
{
	/* Allocate the dorder structure */
	dorder_data_s *od = (dorder_data_s *) safe_malloc_t(dorder_data_s);
	dev->data = od;
	
	/* Initialize */
	parm_next(&parm);
	od->addr = parm_next_uint(&parm);
	od->intno = parm_next_uint(&parm);
	od->cmds = 0;

	/* Checks */

	/* Address alignment */
	if (!addr_word_aligned(od->addr)) {
		mprintf("Dorder address must be 4-byte aligned\n");
		safe_free(od);
		return false;
	}

	/* Address limit */
	if ((uint64_t) od->addr + (uint64_t) REGISTER_LIMIT > 0x100000000ull) {
		mprintf("Invalid address; registers would exceed the 4 GB limit\n");
		return false;
	}

	/* Interrupt number */
	if ((od->intno < 0) || (od->intno > 6)) {
		mprintf("Interrupt number must be within 0..6\n");
		safe_free(od);
		return false;
	}
	
	return true;
}


/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true; always successful
 *
 */
static bool dorder_info(token_t *parm, device_t *dev)
{
	dorder_data_s *od = (dorder_data_s *) dev->data;
	
	mprintf("Address    Int no\n");
	mprintf("---------- ------\n");
	mprintf("%#10" PRIx64 " %d\n", od->addr, od->intno);

	return true;
}


/** Stat comamnd implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true; always successful
 *
 */
static bool dorder_stat(token_t *parm, device_t *dev)
{
	dorder_data_s *od = (dorder_data_s *) dev->data;
	
	mprintf("Command count\n");
	mprintf("--------------------\n");
	mprintf("%" PRIu64 "\n", od->cmds);

	return true;
}


/** Synchup command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true; always successful
 *
 */
static bool dorder_synchup(token_t *parm, device_t *dev)
{
	sync_up_write((dorder_data_s *) dev->data, parm_uint(parm));
	return true;
}


/** Synchdown command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true; always successful
 *
 */
static bool dorder_synchdown(token_t *parm, device_t *dev)
{
	sync_down_write((dorder_data_s *) dev->data, parm_uint(parm));
	return true;
}


/** Clean up the device
 *
 * @param d	Device instance pointer
 *
 */
static void dorder_done(device_t *d)
{
	safe_free(d->name);
	safe_free(d->data);
}


/** Read command implementation
 *
 * @param d    Dorder device pointer
 * @param addr Address of the read operation
 * @param val  Readed (returned) value
 *
 */
static void dorder_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val)
{
	dorder_data_s *od = (dorder_data_s *) dev->data;
	
	if (addr == od->addr + REGISTER_INT_PEND) {
		if (cpu != NULL)
			*val = cpu->procno;
		else
			*val = (uint32_t) -1;
	} else if (addr == od->addr + REGISTER_INT_DOWN)
		*val = 0;
}


/** Write command implementation
 *
 * @param d    Dorder device pointer
 * @param addr Written address
 * @param val  Value to write
 *
 */
void dorder_write(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t val)
{
	dorder_data_s *od = (dorder_data_s *) dev->data;
	
	if (addr == od->addr + REGISTER_INT_UP)
		sync_up_write(od, val);
	else if (addr == od->addr + REGISTER_INT_DOWN)
		sync_down_write(od, val);
}
