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
#include "dcpu.h"
#include "../fault.h"
#include "../parser.h"
#include "../text.h"
#include "../utils.h"

/** \{ \name Registers */
#define REGISTER_INT_UP    0  /**< Assert interrupts */
#define REGISTER_INT_PEND  0  /**< Interrupts pending */
#define REGISTER_INT_DOWN  4  /**< Deassert interrupts */
#define REGISTER_LIMIT     8  /**< Register block size */
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
static void dorder_read32(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t *val);
static void dorder_write32(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t val);

/** Doder object structure */
device_type_t dorder = {
	/* Order device is simulated deterministically */
	.nondet = false,
	
	/* Type name and description */
	.name = id_dorder,
	.brief = "Synchronization device",
	.full =
	    "The order device allows to acquire a unique processor number "
	    "(a serial number) and assert an interrupt to the specified "
	    "processor in the multiprocessor machine.",
	
	/* Functions */
	.done = dorder_done,
	.read32 = dorder_read32,
	.write32 = dorder_write32,
	
	/* Commands */
	.cmds = dorder_cmds
};

/** Dorder instance data structure */
typedef struct {
	ptr36_t addr;        /**< Dorder address */
	unsigned int intno;  /**< Interrupt number */
	
	uint64_t cmds;  /**< Total number of commands */
} dorder_data_s;

/** Write to the synchronisation register - generate interrupts.
 *
 * @param data Instance data structure
 * @param val  Value (mask) which identifies processors
 *
 */
static void sync_up_write(dorder_data_s *data, uint32_t val)
{
	unsigned int i;
	data->cmds++;
	
	for (i = 0; i < 32; i++, val >>= 1)
		if (val & 1)
			dcpu_interrupt_up(i, data->intno);
}

/** Write to the interrupt-down register - disable pending interrupts.
 *
 * @param data Instance data structure
 * @param val  Value (mask) which identifies processors
 *
 */
static void sync_down_write(dorder_data_s *data, uint32_t val)
{
	unsigned int i;
	data->cmds++;
	
	for (i = 0; i < 32; i++, val >>= 1)
		if (val & 1)
			dcpu_interrupt_down(i, data->intno);
}

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dorder_init(token_t *parm, device_t *dev)
{
	parm_next(&parm);
	uint64_t _addr = parm_uint_next(&parm);
	uint64_t _intno = parm_uint_next(&parm);
	
	if (!phys_range(_addr)) {
		error("Physical memory address out of range");
		return false;
	}
	
	if (!phys_range(_addr + (uint64_t) REGISTER_LIMIT)) {
		error("Invalid address, registers would exceed the physical "
		    "memory range");
		return false;
	}
	
	ptr36_t addr = _addr;
	
	if (!ptr36_word_aligned(addr)) {
		error("Physical memory address must by 4-byte aligned");
		return false;
	}
	
	if (_intno > 6) {
		error("%s", txt_intnum_range);
		return false;
	}
	
	/* Allocate the dorder structure */
	dorder_data_s *data = safe_malloc_t(dorder_data_s);
	dev->data = data;
	
	/* Initialize */
	data->addr = addr;
	data->intno = _intno;
	data->cmds = 0;
	
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
	dorder_data_s *data = (dorder_data_s *) dev->data;
	
	printf("[address ] [int]\n");
	printf("%#11" PRIx64 " %-5u\n", data->addr, data->intno);
	
	return true;
}

/** Stat comamnd implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dorder_stat(token_t *parm, device_t *dev)
{
	dorder_data_s *data = (dorder_data_s *) dev->data;
	
	printf("[command count]\n");
	printf("%" PRIu64 "\n", data->cmds);
	
	return true;
}

/** Synchup command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
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
 *
 * @return True (always successful)
 *
 */
static bool dorder_synchdown(token_t *parm, device_t *dev)
{
	sync_down_write((dorder_data_s *) dev->data, parm_uint(parm));
	return true;
}

/** Clean up the device
 *
 * @param dev Device instance pointer
 *
 */
static void dorder_done(device_t *dev)
{
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Read command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dorder_read32(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t *val)
{
	dorder_data_s *data = (dorder_data_s *) dev->data;
	
	switch (addr - data->addr) {
	case REGISTER_INT_PEND:
		if (cpu != NULL)
			*val = cpu->procno;
		else
			*val = (uint32_t) -1;
		break;
	case REGISTER_INT_DOWN:
		*val = 0;
		break;
	}
}

/** Write command implementation
 *
 * @param dev  Dorder device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dorder_write32(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t val)
{
	dorder_data_s *data = (dorder_data_s *) dev->data;
	
	switch (addr - data->addr) {
	case REGISTER_INT_UP:
		sync_up_write(data, val);
		break;
	case REGISTER_INT_DOWN:
		sync_down_write(data, val);
		break;
	}
}
