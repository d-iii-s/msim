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
#include "../text.h"
#include "../fault.h"
#include "../io/output.h"
#include "../utils.h"
#include "../parser.h"

/** Registers */
#define REGISTER_CHAR   0  /**< Output character */
#define REGISTER_LIMIT  4  /**< Size of the register block */

typedef struct {
	ptr_t addr;      /**< Printer register address */
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
	ptr_t addr = parm_next_uint(&parm);
	
	/* Check address alignment */
	if (!addr_word_aligned(addr)) {
		mprintf("Printer address must be on the 4-byte boundary\n");
		return false;
	}
	
	/* The printer structure allocation */
	printer_data_t *data = safe_malloc_t(printer_data_t);
	dev->data = data;
	
	/* Inicialization */
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
	
	mprintf("[Address ]\n");
	mprintf("%#10" PRIx32 "\n", data->addr);
	
	return true;
}

/** Stat command implementation
 *
 */
static bool dprinter_stat(token_t *parm, device_t *dev)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	mprintf("[Count             ]\n");
	mprintf("%20" PRIu64 "\n", data->count);
	
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

/** One step4 implementation
 *
 */
static void printer_step4(device_t *dev)
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
static void printer_write(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t val)
{
	printer_data_t *data = (printer_data_t *) dev->data;
	
	if (addr == data->addr + REGISTER_CHAR) {
		fprintf(data->file, "%c", (char) val);
		data->flush = true;
		data->count++;
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

device_type_s dprinter = {
	/* Type name and description */
	.name = "dprinter",
	.brief = "Printer simulation",
	.full = "Printer device represents a simple character output device. Via "
	    "memory-mapped register system can write character to the "
	    "specified output like screen, file or another terminal.",
	
	/* Functions */
	.done = printer_done,
	.step4 = printer_step4,
	.write = printer_write,
	
	/* Commands */
	printer_cmds
};
