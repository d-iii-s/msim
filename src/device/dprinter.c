/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Printer device
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "dprinter.h"
#include "device.h"
#include "../assert.h"
#include "../fault.h"
#include "../parser.h"
#include "../text.h"
#include "../utils.h"

/** Registers */
#define REGISTER_CHAR   0  /**< Output character */
#define REGISTER_LIMIT  4  /**< Size of the register block */

typedef struct {
	ptr36_t addr;    /**< Printer register address */
	bool flush;      /**< Flush-the-output flag */
	
	FILE *file;      /**< Output file */
	char *fname;     /**< Output file name */
	
	uint64_t count;  /**< Number of printed characters */
} printer_data_t;

/** Init command implementation
 *
 */
static bool dprinter_init(token_t *parm, device_t *dev)
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
	
	/* The printer structure allocation */
	printer_data_t *data = safe_malloc_t(printer_data_t);
	dev->data = data;
	
	/* Initialization */
	data->addr = addr;
	data->flush = false;
	data->file = stdout;
	data->fname = NULL;
	data->count = 0;
	
	return true;
}

/** Redir command implementation
 *
 */
static bool dprinter_redir(token_t *parm, device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	char *fname = parm_str(parm);
	
	/* Open the file */
	FILE *file = try_fopen(fname, "w");
	if (!file)
		return false;
	
	/* Close old output file */
	if (data->file != stdout) {
		safe_fclose(data->file, data->fname);
		safe_free(data->fname);
	}
	
	/* Set new output file */
	data->file = file;
	data->fname = safe_strdup(fname);
	return true;
}

/** Stdout command implementation
 *
 */
static bool dprinter_stdout(token_t *parm, device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	/* Close old ouput file if it is not stdout already */
	if (data->file != stdout) {
		safe_fclose(data->file, data->fname);
		safe_free(data->fname);
		data->file = stdout;
	}
	
	return true;
}

/** Info command implementation
 *
 */
static bool dprinter_info(token_t *parm, device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	printf("[address ]\n");
	printf("%#11" PRIx64 "\n", data->addr);
	
	return true;
}

/** Stat command implementation
 *
 */
static bool dprinter_stat(token_t *parm, device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	printf("[count             ]\n");
	printf("%20" PRIu64 "\n", data->count);
	
	return true;
}

/** Clean up the device
 *
 */
static void printer_done(device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	/* Close output file if it is not stdout */
	if (data->file != stdout) {
		safe_fclose(data->file, data->fname);
		safe_free(data->fname);
	}
	
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Printer implementation
 *
 */
static void printer_step4k(device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	/* Check if flush is necesary */
	if (data->flush) {
		data->flush = false;
		fflush(data->file);
	}
}

/** Write command implementation
 *
 */
static void printer_write32(r4k_cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t val)
{
	ASSERT(dev != NULL);
	
	printer_data_t *data = (printer_data_t *) dev->data;
	
	switch (addr - data->addr) {
	case REGISTER_CHAR:
		fprintf(data->file, "%c", (char) val);
		data->flush = true;
		data->count++;
		break;
	}
}

/*
 * Device commands
 */

static cmd_t printer_cmds[] = {
	{
		"init",
		(fcmd_t) dprinter_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/printer name" NEXT
		REQ INT "addr/register address" END
	},
	{
		"help",
		(fcmd_t) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display this help text",
		"Display this help text",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(fcmd_t) dprinter_info,
		DEFAULT,
		DEFAULT,
		"Display printer state and configuration",
		"Display printer state and configuration",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dprinter_stat,
		DEFAULT,
		DEFAULT,
		"Display printer statistics",
		"Display printer statistics",
		NOCMD
	},
	{
		"redir",
		(fcmd_t) dprinter_redir,
		DEFAULT,
		DEFAULT,
		"Redirect output to the specified file",
		"Redirect output to the specified file",
		REQ STR "filename/output file name" END
	},
	{
		"stdout",
		(fcmd_t) dprinter_stdout,
		DEFAULT,
		DEFAULT,
		"Redirect output to the standard output",
		"Redirect output to the standard output",
		NOCMD
	},
	LAST_CMD
};

device_type_t dprinter = {
	/* Printer is simulated deterministically */
	.nondet = false,
	
	/* Type name and description */
	.name = "dprinter",
	.brief = "Printer simulation",
	.full = "Printer device represents a simple character output device. Via "
	    "memory-mapped register system can write character to the "
	    "specified output like screen, file or another terminal.",
	
	/* Functions */
	.done = printer_done,
	.step4k = printer_step4k,
	.write32 = printer_write32,
	
	/* Commands */
	.cmds = printer_cmds
};
