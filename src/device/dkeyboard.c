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
#include "dcpu.h"
#include "../arch/stdin.h"
#include "../fault.h"
#include "../utils.h"

/* Register offsets */
#define REGISTER_CHAR   0
#define REGISTER_LIMIT  4

/* Step skip constant */
#define CYCLE_SKIP  4096

/*
 * Device commands
 */

static bool dkeyboard_init(token_t *parm, device_t *dev);
static bool dkeyboard_info(token_t *parm, device_t *dev);
static bool dkeyboard_stat(token_t *parm, device_t *dev);
static bool dkeyboard_gen(token_t *parm, device_t *dev);

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

const char id_keyboard[] = "dkeyboard";

static void keyboard_done(device_t *dev);
static void keyboard_step4(device_t *dev);
static void keyboard_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val);

device_type_s dkeyboard = {
	/* Type name and description */
	.name = id_keyboard,
	.brief = "Keyboard simulation",
	.full = "Device reads key codes from the specified input and sends them to "
		"the system via the interrrupt assert and a memory-mapped "
		"register containing a key code.",
	
	/* Functions */
	.done = keyboard_done,
	.step4 = keyboard_step4,
	.read = keyboard_read,

	/* Commands */
	.cmds = keyboard_cmds
};

struct keyboard_data_s {
	uint32_t addr;		/* Dkeyboard register address. */
	int intno;		/* Interrupt number */
	char incomming;		/* Character buffer */
	
	bool ig;		/* Interrupt pending flag */
	uint64_t intrcount;		/* Number of interrupts asserted */
	uint64_t keycount;		/* Number of keys acquired */
	uint64_t overrun;		/* Number of overwritten characters in the buffer. */
};
typedef struct keyboard_data_s keyboard_data_s;


/** Generate a key press
 *
 * An interrupt is asserted.
 *
 */
static void gen_key(device_t *dev, char k)
{
	keyboard_data_s *kd = (keyboard_data_s *) dev->data;

	kd->incomming = k;
	kd->keycount++;
			
	if (!kd->ig) {
		kd->ig = true;
		kd->intrcount++;
		dcpu_interrupt_up(0, kd->intno);
	} else
		/* Increase the number of overrun characters */
		kd->overrun++;
}


/** Init command implementation
 *
 */
static bool dkeyboard_init(token_t *parm, device_t *dev)
{
	/* Alloc structure */
	keyboard_data_s *kd = (keyboard_data_s *) safe_malloc_t(keyboard_data_s);
	dev->data = kd;
	
	/* Initialization */
	parm_next( &parm);
	kd->addr = parm_next_uint(&parm);
	kd->intno = parm_next_uint(&parm);
	
	kd->ig = false;
	kd->intrcount = 0;
	kd->keycount = 0;
	kd->overrun = 0;

	/* Checks */

	/* Address alignment */
	if (!addr_word_aligned(kd->addr)) {
		error("Keyboard address must be on 4-byte aligned");
		free(kd);
		return false;
	}

	/* Interrupt no */
	if (kd->intno > 6) {
		error("Interrupt number must be within 0..6");
		free(kd);
		return false;
	}

	return true;
}


/** Info command implementation
 *
 */
static bool dkeyboard_info(token_t *parm, device_t *dev)
{
	keyboard_data_s *kb = (keyboard_data_s *) dev->data;
	
	printf("[Address ] [Int no] [Key] [Ig  ]\n");
	printf("%#08x %-8u %#02x  %u\n",
	    kb->addr, kb->intno, kb->incomming, kb->ig);
	
	return true;
}


/** Stat command implementation
 *
 */
static bool dkeyboard_stat(token_t *parm, device_t *dev)
{
	keyboard_data_s *kd = (keyboard_data_s *) dev->data;
	
	printf("[Interrupt count   ] [Key count         ] [Overrun           ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n",
	    kd->intrcount, kd->keycount, kd->overrun);
	
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
	unsigned char c;

	/* Parameter is string - take the first character */
	if (parm_type( parm) == tt_str) {
		c = parm_str(parm)[ 0];

		if ((!c) || (parm_str(parm)[ 1])) {
			error("Invalid key (must be exactly one character)");
			return false;
		}
	} else {
		/* Parameter is integer - interpret is as ASCII value. */
		if (parm_uint(parm) > 255) {
			error("Invalid key (must be within 0..255)");
			return false;
		}

		c = parm_uint(parm);
	}

	gen_key(dev, c);
	
	return true;
}


/** Clean up the device
 *
 */
static void keyboard_done(device_t *d)
{
	safe_free(d->name);
	safe_free(d->data);
}
	

/** Read implementation
 *
 * The read command returns a character in the buffer. Any pending interrupt is
 * deasserted.
 *
 */
static void keyboard_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val)
{
	keyboard_data_s *kd = (keyboard_data_s *) dev->data;
	
	if (addr == kd->addr + REGISTER_CHAR) {
		*val = kd->incomming;
		kd->incomming = 0;
		if (kd->ig) {
			kd->ig = false;
			dcpu_interrupt_down(0, kd->intno);
		}
	}
}


/** One step4 implementation
 *
 */
static void keyboard_step4(device_t *dev)
{
	char buf;
	
	if (stdin_poll(&buf))
		gen_key(dev, buf);
}
