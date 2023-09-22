/*
 * Copyright (c) 2023 Vojtech Horky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Debug no-memory device. Writing to this area causes simulator to enter
 * interactive mode.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "dnomem.h"
#include "../fault.h"
#include "../utils.h"

/** Access handling */
enum nomem_access_handling_e {
	NOMEM_ACCESS_WARN,
	NOMEM_ACCESS_ENTER_DEBUGGER,
	NOMEM_ACCESS_HALT_MACHINE,
};

/** Device no-mem instance data structure */
typedef struct {
	/** No-memory location. */
	ptr36_t addr;
	/** No-memory size. */
	uint64_t size;
	/** No-memory behavior on access. */
	enum nomem_access_handling_e mode;
} dnomem_data_s;


/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dnomem_init(token_t *parm, device_t *dev)
{
	parm_next(&parm);
	uint64_t start_addr = parm_uint_next(&parm);
	uint64_t size = parm_uint_next(&parm);

	if (!phys_range(start_addr)) {
		error("Physical memory address out of range");
		return false;
	}

	if (!phys_range(start_addr + size)) {
		error("Invalid address, size would exceed the physical "
		    "memory range");
		return false;
	}

	if (!ptr36_dword_aligned(start_addr)) {
		error("Physical memory address must be 8-byte aligned");
		return false;
	}

	/* Allocate structure */
	dnomem_data_s *data = safe_malloc_t(dnomem_data_s);
	dev->data = data;

	/* Basic structure inicialization */
	data->addr = start_addr;
	data->size = size;
	data->mode = NOMEM_ACCESS_WARN;

	return true;
}

/** mode command implementation.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 */
static bool dnomem_setmode(token_t *parm, device_t *dev)
{
	dnomem_data_s *data = (dnomem_data_s *) dev->data;
	const char *const mode = parm_str(parm);

	if (strcmp(mode, "warn") == 0) {
		data->mode = NOMEM_ACCESS_WARN;
	} else if (strcmp(mode, "debug") == 0) {
		data->mode = NOMEM_ACCESS_ENTER_DEBUGGER;
	} else if (strcmp(mode, "halt") == 0) {
		data->mode = NOMEM_ACCESS_HALT_MACHINE;
	} else {
		error("Unknown mode %s, expecting warn, debug or halt.", mode);
		return false;
	}

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
static bool dnomem_info(token_t *parm, device_t *dev)
{
	dnomem_data_s *data = (dnomem_data_s *) dev->data;

	printf("[address  ] [size       ]\n"
	    "%#011" PRIx64 " %#011" PRIx64 "\n",
	    data->addr, data->size);

	return true;
}


/** Dispose disk
 *
 * @param dev Device pointer
 *
 */
static void dnomem_done(device_t *dev) {
	safe_free(dev->name);
	safe_free(dev->data);
}

static void dnomem_access32(const char *operation_name, device_t *dev, ptr36_t addr)
{
	dnomem_data_s *data = (dnomem_data_s *) dev->data;
	ptr36_t offset = addr - data->addr;
	if (offset >= data->size) {
		return;
	}

	switch (data->mode) {
	case NOMEM_ACCESS_WARN:
		alert("Ignoring %s (at %#011" PRIx64 ", %#" PRIx64 " inside %s).", operation_name, addr, offset, dev->name);
		break;
	case NOMEM_ACCESS_ENTER_DEBUGGER:
		alert("Entering interactive mode because of invalid %s (at %#011" PRIx64 ", %#" PRIx64 " inside %s),",
				operation_name, addr, offset, dev->name);
		machine_interactive = true;
		break;
	case NOMEM_ACCESS_HALT_MACHINE:
		alert("Halting after forbidden %s (at %#011" PRIx64 ", %#" PRIx64 " inside %s).",
				operation_name, addr, offset, dev->name);
		machine_halt = true;
		break;
	}
}

/** Read command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dnomem_read32(unsigned int procno, device_t *dev, ptr36_t addr,
    uint32_t *val)
{
	dnomem_access32("READ", dev, addr);
}

/** Write command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dnomem_write32(unsigned int procno, device_t *dev, ptr36_t addr,
    uint32_t val)
{
	dnomem_access32("WRITE", dev, addr);
}


cmd_t dnomem_cmds[] = {
	{
		"init",
		(fcmd_t) dnomem_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/nomem name" NEXT
		REQ INT "addr/start address" NEXT
		REQ INT "size/size of the memory" END
	},
	{
		"mode",
		(fcmd_t) dnomem_setmode,
		DEFAULT,
		DEFAULT,
		"Set mode",
		"Set mode",
		REQ STR "mode/Mode name (warn, debug, halt)" END
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
		(fcmd_t) dnomem_info,
		DEFAULT,
		DEFAULT,
		"Configuration information",
		"Configuration information",
		NOCMD
	},
	LAST_CMD
};

/**< Dnomem object structure */
device_type_t dnomem = {
	.nondet = false,

	.name = "dnomem",
	.brief = "No-memory area",
	.full = "Enters interactive mode when accessed",

	.done = dnomem_done,
	.step = NULL,
	.read32 = dnomem_read32,
	.write32 = dnomem_write32,

	/* Commands */
	.cmds = dnomem_cmds
};
