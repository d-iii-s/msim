/*
 * Copyright (c) 2010 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Time device
 *
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "dcycle.h"
#include "device.h"
#include "../assert.h"
#include "../fault.h"
#include "../utils.h"

/** Registers */
#define REGISTER_CYCLE_LO  0
#define REGISTER_CYCLE_HI  4
#define REGISTER_LIMIT     8

/** Instance data structure */
typedef struct {
	ptr36_t addr;
	uint64_t cycle;
} dcycle_data_t;

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dcycle_init(token_t *parm, device_t *dev)
{
	parm_next(&parm);
	uint64_t _addr = parm_uint(parm);
	
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
	
	if (!ptr36_dword_aligned(addr)) {
		error("Physical memory address must be 8-byte aligned");
		return false;
	}
	
	/* Initialization */
	dcycle_data_t *data = safe_malloc_t(dcycle_data_t);
	dev->data = data;
	
	data->addr = addr;
	data->cycle = 0;
	
	return true;
}

/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dcycle_info(token_t *parm, device_t *dev)
{
	dcycle_data_t *data = (dcycle_data_t *) dev->data;
	
	printf("[address ]\n");
	printf("%#11" PRIx64 "\n", data->addr);
	
	return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dcycle_stat(token_t *parm, device_t *dev)
{
	dcycle_data_t *data = (dcycle_data_t *) dev->data;
	
	printf("[cycle              ]\n");
	printf("%20" PRIu64 "\n", data->cycle);
	
	return true;
}

/** Dispose device
 *
 * @param dev Device pointer
 *
 */
static void dcycle_done(device_t *dev)
{
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Read command implementation (32 bits)
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dcycle_read32(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t *val)
{
	ASSERT(dev != NULL);
	ASSERT(val != NULL);
	
	dcycle_data_t *data = (dcycle_data_t *) dev->data;
	
	switch (addr - data->addr) {
	case REGISTER_CYCLE_LO:
		*val = (uint32_t) data->cycle;
		break;
	case REGISTER_CYCLE_HI:
		*val = (uint32_t) (data->cycle >> 32);
		break;
	}
}

/** Read command implementation (64 bits)
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dcycle_read64(cpu_t *cpu, device_t *dev, ptr36_t addr, uint64_t *val)
{
	ASSERT(dev != NULL);
	ASSERT(val != NULL);
	
	dcycle_data_t *data = (dcycle_data_t *) dev->data;
	
	switch (addr - data->addr) {
	case REGISTER_CYCLE_LO:
		*val = data->cycle;
		break;
	}
}

/** Count one processor step
 *
 */
static void dcycle_step(device_t *dev)
{
	ASSERT(dev != NULL);
	
	dcycle_data_t *data = (dcycle_data_t *) dev->data;
	data->cycle++;
}

static cmd_t dcycle_cmds[] = {
	{
		"init",
		(fcmd_t) dcycle_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/cycle device name" NEXT
		REQ INT "addr/cycle device register address" END
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
		(fcmd_t) dcycle_info,
		DEFAULT,
		DEFAULT,
		"Display device configuration",
		"Display device configuration",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dcycle_stat,
		DEFAULT,
		DEFAULT,
		"Display device statictics",
		"display device statictics",
		NOCMD
	},
	LAST_CMD
};

/** Dtime object structure */
device_type_t dcycle = {
	.nondet = false,
	
	/* Type name and description */
	.name = "dcycle",
	.brief = "Cycle device",
	.full = "Device for reading a 64-bit CPU cycle counter.",
	
	/* Functions */
	.done = dcycle_done,
	.read32 = dcycle_read32,
	.read64 = dcycle_read64,
	.step = dcycle_step,
	
	/* Commands */
	.cmds = dcycle_cmds
};
