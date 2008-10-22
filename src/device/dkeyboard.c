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

#include "../include/select.h"
#include "dkeyboard.h"
#include "device.h"
#include "dcpu.h"
#include "../io/output.h"
#include "../utils.h"

/* Register offsets */
#define REGISTER_CHAR  0
#define REGISTER_LIMIT 4

/* Step skip constant */
#define CYCLE_SKIP 4096

/*
 * Device commands
 */

static bool dkeyboard_init(parm_link_s *parm, device_s *dev);
static bool dkeyboard_info(parm_link_s *parm, device_s *dev);
static bool dkeyboard_stat(parm_link_s *parm, device_s *dev);
static bool dkeyboard_gen(parm_link_s *parm, device_s *dev);

cmd_s keyboard_cmds[] = {
	{
		"init",
		(cmd_f) dkeyboard_init,
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
		(cmd_f) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display this help text",
		"Display this help text",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(cmd_f) dkeyboard_info,
		DEFAULT,
		DEFAULT,
		"Display keyboard state and configuration",
		"Display keyboard state and configuration",
		NOCMD
	},
	{
		"stat",
		(cmd_f) dkeyboard_stat,
		DEFAULT,
		DEFAULT,
		"Display keyboard statistics",
		"Display keyboard statistics",
		NOCMD
	},
	{
		"gen",
		(cmd_f) dkeyboard_gen,
		DEFAULT,
		DEFAULT,
		"Generate a key press with specified code",
		"Generate a key press with specified code",
		REQ VAR "key code" END
	},
	LAST_CMD
};

const char id_keyboard[] = "dkeyboard";

static void keyboard_done(device_s *dev);
static void keyboard_step4(device_s *dev);
static void keyboard_read(processor_t *pr, device_s *dev, addr_t addr, uint32_t *val);

device_type_s DKeyboard = {
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
	long intrcount;		/* Number of interrupts asserted */
	long keycount;		/* Number of keys acquired */
	long overrun;		/* Number of overwritten characters in the buffer. */
};
typedef struct keyboard_data_s keyboard_data_s;


/** Generate a key press
 *
 * An interrupt is asserted.
 *
 */
static void gen_key(device_s *dev, char k)
{
	keyboard_data_s *kd = dev->data;

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
static bool dkeyboard_init(parm_link_s *parm, device_s *dev)
{
	keyboard_data_s *kd;
	
	/* Alloc structure */
	kd = safe_malloc_t(keyboard_data_s);
	dev->data = kd;
	
	/* Initialization */
	parm_next( &parm);
	kd->addr = parm_next_int(&parm);
	kd->intno = parm_next_int(&parm);
	
	kd->ig = false;
	kd->intrcount = 0;
	kd->keycount = 0;
	kd->overrun = 0;

	/* Checks */

	/* Address alignment */
	if (!addr_word_aligned(kd->addr)) {
		mprintf("Keyboard address must be on 4-byte aligned.\n");
		free(kd);
		return false;
	}

	/* Interrupt no */
	if (kd->intno > 6) {
		mprintf("Interrupt number must be within 0..6.\n");
		free(kd);
		return false;
	}

	return true;
}


/** Info command implementation
 *
 */
static bool dkeyboard_info(parm_link_s *parm, device_s *dev)
{
	keyboard_data_s *kd = dev->data;
	
	mprintf("address:0x%08x intno:%d regs(key:0x%02x ig:%d)\n",
			kd->addr, kd->intno, kd->incomming, kd->ig);
	
	return true;
}


/** Stat command implementation
 *
 */
static bool dkeyboard_stat(parm_link_s *parm, device_s *dev)
{
	keyboard_data_s *kd = dev->data;
	
	mprintf("intrc:%ld keycount:%ld overrun:%ld\n",
			kd->intrcount, kd->keycount, kd->overrun);
	
	return true;
}


/** Gen command implementation
 *
 * The gen command allows two types of the
 * argument - a character or an integer.
 *
 */
static bool dkeyboard_gen(parm_link_s *parm, device_s *dev)
{
	unsigned char c;

	/* Parameter is string - take the first character */
	if (parm_type( parm) == tt_str) {
		c = parm_str(parm)[ 0];

		if ((!c) || (parm_str(parm)[ 1])) {
			mprintf("Invalid key (must be exactly one character).\n");
			return false;
		}
	} else {
		/* Parameter is integer - interpret is as ASCII value. */
		if (parm_int(parm) > 255) {
			mprintf("Invalid key (must be within 0..255).\n");
			return false;
		}

		c = parm_int(parm);
	}

	gen_key(dev, c);
	
	return true;
}


/** Clean up the device
 *
 */
static void keyboard_done(device_s *d)
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
static void keyboard_read(processor_t *pr, device_s *dev, addr_t addr, uint32_t *val)
{
	keyboard_data_s *kd = dev->data;
	
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
static void keyboard_step4(device_s *dev)
{
	/* Check new character */
	fd_set rfds;
	struct timeval tv;
	int retval;
	char buf;

	/* Watch stdin */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval == 1) {
		/* There is a new character. */
		read(0, &buf, 1);
		gen_key(dev, buf);
	}
}
