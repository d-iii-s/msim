/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Device infrastructure
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>
#include <inttypes.h>
#include "dkeyboard.h"
#include "device.h"
#include "cpu/general_cpu.h"
#include "../arch/stdin.h"
#include "../assert.h"
#include "../env.h"
#include "../fault.h"
#include "../text.h"
#include "../utils.h"

/* Register offsets */
#define REGISTER_CHAR   0
#define REGISTER_LIMIT  4

typedef struct {
	ptr36_t addr;        /* Register address */
	unsigned int intno;  /* Interrupt number */
	char incomming;      /* Character buffer */

	bool ig;             /* Interrupt pending flag */
	uint64_t intrcount;  /* Number of interrupts asserted */
	uint64_t keycount;   /* Number of keys acquired */
	uint64_t overrun;    /* Number of overwritten characters in the buffer. */
} keyboard_data_s;

/** Generate a key press
 *
 * An interrupt is asserted.
 *
 */
static void gen_key(device_t *dev, char c)
{
	keyboard_data_s *data = (keyboard_data_s *) dev->data;

	//TODO: Generate SC check?
	data->incomming = c;
	data->keycount++;

	if (!data->ig) {
		data->ig = true;
		data->intrcount++;
		cpu_interrupt_up(NULL, data->intno);
	} else
		/* Increase the number of overrun characters */
		data->overrun++;
}

/** Init command implementation
 *
 */
static bool dkeyboard_init(token_t *parm, device_t *dev)
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

	if (!ptr36_dword_aligned(addr)) {
		error("Physical memory address must be 8-byte aligned");
		return false;
	}

	if (_intno > MAX_INTRS) {
		error("%s", txt_intnum_range);
		return false;
	}

	/* Alloc structure */
	keyboard_data_s *data = safe_malloc_t(keyboard_data_s);
	dev->data = data;

	/* Initialization */
	data->addr = addr;
	data->intno = _intno;
	data->ig = false;
	data->intrcount = 0;
	data->keycount = 0;
	data->overrun = 0;

	return true;
}

/** Info command implementation
 *
 */
static bool dkeyboard_info(token_t *parm, device_t *dev)
{
	keyboard_data_s *data = (keyboard_data_s *) dev->data;

	printf("[address ] [int] [key] [ig]\n");
	printf("%#11" PRIx64 " %-5u %#02x  %u\n",
	    data->addr, data->intno, data->incomming, data->ig);

	return true;
}

/** Stat command implementation
 *
 */
static bool dkeyboard_stat(token_t *parm, device_t *dev)
{
	keyboard_data_s *data = (keyboard_data_s *) dev->data;

	printf("[interrupt count   ] [key count         ] [overrun           ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n",
	    data->intrcount, data->keycount, data->overrun);

	return true;
}

/** Gen command implementation
 *
 * The gen command allows two types of the
 * argument - a character or an integer.
 *
 */
static bool dkeyboard_gen(token_t *parm, device_t *dev)
{
	const char *str;
	char c = 0;

	switch (parm_type(parm)) {
	case tt_end:
		/* default '\0' */
		break;
	case tt_str:
		str = parm_str(parm);
		c = str[0];

		if ((!c) || (str[1])) {
			error("Invalid character");
			return false;
		}

		break;
	case tt_uint:
		if (parm_uint(parm) > 255) {
			error("Integer out of range 0..255");
			return false;
		}

		c = parm_uint(parm);
		break;
	default:
		intr_error("Unexpected parameter type");
		return false;
	}

	gen_key(dev, c);
	return true;
}

/** Clean up the device
 *
 */
static void keyboard_done(device_t *dev)
{
	safe_free(dev->data);
}

static void keyboard_read32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t *val)
{
	ASSERT(dev != NULL);
	ASSERT(val != NULL);

	keyboard_data_s *data = (keyboard_data_s *) dev->data;

	switch (addr - data->addr) {
	case REGISTER_CHAR:
		*val = data->incomming;
		data->incomming = 0;
		if (data->ig) {
			data->ig = false;
			cpu_interrupt_down(NULL, data->intno);
		}
		break;
	}
}

/** Keyboard implementation
 *
 */
static void keyboard_step4k(device_t *dev)
{
	char c;

	if (stdin_poll(&c))
		gen_key(dev, c);
}

/*
 * Device commands
 */

cmd_t keyboard_cmds[] = {
	{
		"init",
		(fcmd_t) dkeyboard_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "keyboard name" NEXT
		REQ INT "register address" NEXT
		REQ INT "interrupt number" END
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
		(fcmd_t) dkeyboard_info,
		DEFAULT,
		DEFAULT,
		"Display keyboard state and configuration",
		"Display keyboard state and configuration",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dkeyboard_stat,
		DEFAULT,
		DEFAULT,
		"Display keyboard statistics",
		"Display keyboard statistics",
		NOCMD
	},
	{
		"gen",
		(fcmd_t) dkeyboard_gen,
		DEFAULT,
		DEFAULT,
		"Generate a key press with specified code",
		"Generate a key press with specified code",
		REQ VAR "key code" END
	},
	LAST_CMD
};

device_type_t dkeyboard = {
	.nondet = true,

	/* Type name and description */
	.name = "dkeyboard",
	.brief = "Keyboard simulation",
	.full = "Device reads key codes from the specified input and sends them to "
	    "the system via the interrrupt assert and a memory-mapped "
	    "register containing a key code.",

	/* Functions */
	.done = keyboard_done,
	.step4k = keyboard_step4k,
	.read32 = keyboard_read32,

	/* Commands */
	.cmds = keyboard_cmds
};
