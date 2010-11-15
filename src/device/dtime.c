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
#include "../io/output.h"
#include "../check.h"
#include "../utils.h"

/** Registers */
#define REGISTER_SEC    0
#define REGISTER_USEC   4
#define REGISTER_LIMIT  8

/** Dtime instance data structure */
typedef struct {
	uint32_t addr;  /**< Memory location */
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
	/* Checks */
	parm_next(&parm);
	ptr_t addr = parm_next_uint(&parm);
	
	/* Address alignment */
	if (!addr_word_aligned(addr)) {
		mprintf("Dtime address must be 4-byte aligned\n");
		return false;
	}
	
	/* Address limit */
	if ((uint64_t) addr + (uint64_t) REGISTER_LIMIT > 0x100000000ull) {
		mprintf("Invalid address; registers would exceed the 4 GB limit\n");
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
	
	mprintf("[Address ]\n");
	mprintf("%#10" PRIx32 "\n", data->addr);
	
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
	mprintf("No statistics\n");
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

/** Read command implementation
 *
 * Read host time via gettimeofday().
 *
 * @param dev  Ddisk device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dtime_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val)
{
	PRE(val != NULL);
	
	dtime_data_t *data = (dtime_data_t *) dev->data;
	
	/* Get actual time */
	struct timeval timeval;
	gettimeofday(&timeval, NULL);
	
	if (addr == data->addr + REGISTER_SEC)
		*val = (uint32_t) timeval.tv_sec;
	
	if (addr == data->addr + REGISTER_USEC)
		*val = (uint32_t) timeval.tv_usec;
}

static cmd_t dtime_cmds[] = {
	{
		"init",
		(fcmd_t) dtime_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "time/timer name" NEXT
		REQ INT "arrd/timer register address" END
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
device_type_s dtime = {
	/* Type name and description */
	.name = "dtime",
	.brief = "Real time",
	.full = "The time device brings the host real time to the simulated "
	    "environment. One memory-mapped register allows programs"
	    "to read hosts time since the Epoch as specified in the"
	    "POSIX.",
	
	/* Functions */
	.done = dtime_done,
	.read = dtime_read,
	
	/* Commands */
	.cmds = dtime_cmds
};
