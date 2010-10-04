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
#include "../utils.h"

/** \{ \name Register offsets */
#define REGISTER_SEC   0
#define REGISTER_USEC  4
#define REGISTER_LIMIT 8
/* \} */


/*
 * Device commands
 */

static bool dtime_init(parm_link_s *parm, device_s *dev);
static bool dtime_info(parm_link_s *parm, device_s *dev);
static bool dtime_stat(parm_link_s *parm, device_s *dev);

cmd_s dtime_cmds[] = {
	{
		"init",
		(cmd_f) dtime_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "time/timer name" NEXT
		REQ INT "arrd/timer register address" END
	},
	{
		"help",
		(cmd_f) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display help",
		"Display help",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(cmd_f) dtime_info,
		DEFAULT,
		DEFAULT,
		"Display device configuration",
		"Display device configuration",
		NOCMD
	},
	{
		"stat",
		(cmd_f) dtime_stat,
		DEFAULT,
		DEFAULT,
		"Display device statictics",
		"display device statictics",
		NOCMD
	},
	LAST_CMD
};

/** Name of the dtime as presented to the user */
const char id_dtime[] = "dtime";

static void dtime_done(device_s *dev);
static void dtime_read(cpu_t *cpu, device_s *dev, ptr_t addr, uint32_t *val);

/** Dtime object structure */
device_type_s dtime = {
	/* Type name and description */
	.name = id_dtime,
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


/** Dtime instance data structure */
typedef struct {
	uint32_t addr;	/**< Dtime memory location */
} dtime_data_s;

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true if successful
 *
 */
static bool dtime_init(parm_link_s *parm, device_s *dev)
{
	/* Alloc the dtime structure. */
	dtime_data_s *td = (dtime_data_s *) safe_malloc_t(dtime_data_s);
	dev->data = td;
	
	/* Inicialization */
	parm_next(&parm);
	td->addr = parm_next_int(&parm);
	
	/* Checks */

	/* Address alignment */
	if (!addr_word_aligned(td->addr)) {
		mprintf("Dtime address must be 4-byte aligned\n");
		free(td);
		return false;
	}

	/* Address limit */
	if ((uint64_t) td->addr + (uint64_t) REGISTER_LIMIT > 0x100000000ull) {
		mprintf("Invalid address; registers would exceed the 4 GB limit\n");
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
static bool dtime_info(parm_link_s *parm, device_s *dev)
{
	dtime_data_s *td = (dtime_data_s *) dev->data;
	
	mprintf("Address\n");
	mprintf("----------\n");
	mprintf("%#10" PRIx64 "\n", td->addr);
	
	return true;
}


/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true; always successful
 *
 */
static bool dtime_stat(parm_link_s *parm, device_s *dev)
{
	mprintf("No statistics\n");
	return true;
}


/** Dispose dtime
 *
 * @param d Device pointer
 *
 */
static void dtime_done(device_s *d)
{
	safe_free(d->name);
	safe_free(d->data);
}


/** Read command implementation
 *
 * Read host time via gettimeofday().
 *
 * @param d    Ddisk device pointer
 * @param addr Address of the read operation
 * @param val  Readed (returned) value
 *
 */
static void dtime_read(cpu_t *cpu, device_s *dev, ptr_t addr, uint32_t *val)
{
	dtime_data_s *od = (dtime_data_s *) dev->data;
	
	if ((addr == od->addr + REGISTER_SEC) || (addr == od->addr + REGISTER_USEC)) {
		/* Get actual time */
		struct timeval t;
		gettimeofday( &t, NULL);
		*val = (addr == od->addr) ? (uint32_t) t.tv_sec : (uint32_t) t.tv_usec;
	}
}
