/*
 * Copyright (c) 2003-2007 Viliam Holub
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
#include <stdbool.h>
#include <sys/time.h>
#include <inttypes.h>
#include "dtime.h"
#include "device.h"
#include "../assert.h"
#include "../fault.h"
#include "../utils.h"

// TODO: Add SC checks on each change of value?

/** Registers */
#define REGISTER_SEC    0
#define REGISTER_USEC   4
#define REGISTER_LIMIT  8

/** Dtime instance data structure */
typedef struct {
	ptr36_t addr;  /**< Memory location */
} dtime_data_t;

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dtime_init(token_t *parm, device_t *dev)
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
	dtime_data_t *data = safe_malloc_t(dtime_data_t);
	dev->data = data;
	
	data->addr = addr;
	
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
static bool dtime_info(token_t *parm, device_t *dev)
{
	dtime_data_t *data = (dtime_data_t *) dev->data;
	
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
static bool dtime_stat(token_t *parm, device_t *dev)
{
	printf("No statistics\n");
	return true;
}

/** Dispose dtime
 *
 * @param dev Device pointer
 *
 */
static void dtime_done(device_t *dev)
{
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Read command implementation (32 bits)
 *
 * Read host time via gettimeofday().
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dtime_read32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t *val)
{
	ASSERT(dev != NULL);
	ASSERT(val != NULL);
	
	dtime_data_t *data = (dtime_data_t *) dev->data;
	
	/* Get actual time */
	struct timeval timeval;
	
	switch (addr - data->addr) {
	case REGISTER_SEC:
		gettimeofday(&timeval, NULL);
		*val = (uint32_t) timeval.tv_sec;
		break;
	case REGISTER_USEC:
		gettimeofday(&timeval, NULL);
		*val = (uint32_t) timeval.tv_usec;
		break;
	}
}

/** Read command implementation (64 bits)
 *
 * Read host time via gettimeofday().
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dtime_read64(unsigned int procno, device_t *dev, ptr36_t addr, uint64_t *val)
{
	ASSERT(dev != NULL);
	ASSERT(val != NULL);
	
	dtime_data_t *data = (dtime_data_t *) dev->data;
	
	/* Get actual time */
	struct timeval timeval;
	uint32_t sec = (uint32_t) timeval.tv_sec;
	uint32_t usec = (uint32_t) timeval.tv_usec;
	
	/* Pack the values in little-endian fashion */
	switch (addr - data->addr) {
	case REGISTER_SEC:
		gettimeofday(&timeval, NULL);
		*val = ((uint64_t) sec) | ((uint64_t) usec << 32);
		break;
	}
}

static cmd_t dtime_cmds[] = {
	{
		"init",
		(fcmd_t) dtime_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/timer name" NEXT
		REQ INT "addr/timer register address" END
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
		(fcmd_t) dtime_info,
		DEFAULT,
		DEFAULT,
		"Display device configuration",
		"Display device configuration",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dtime_stat,
		DEFAULT,
		DEFAULT,
		"Display device statictics",
		"display device statictics",
		NOCMD
	},
	LAST_CMD
};

/** Dtime object structure */
device_type_t dtime = {
	/* Real-time device induces non-determinism */
	.nondet = true,
	
	/* Type name and description */
	.name = "dtime",
	.brief = "Real-time",
	.full = "The time device brings the host real time to the simulated "
	    "environment. One memory-mapped register allows programs"
	    "to read hosts time since the Epoch as specified in the"
	    "POSIX.",
	
	/* Functions */
	.done = dtime_done,
	.read32 = dtime_read32,
	.read64 = dtime_read64,
	
	/* Commands */
	.cmds = dtime_cmds
};
