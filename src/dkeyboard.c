/*
 * Keyboard device
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#include "dkeyboard.h"

#include "device.h"
#include "dcpu.h"
#include "output.h"
#include "utils.h"


/*
 * Device commands
 */

static bool dkeyboard_init( parm_link_s *parm, device_s *dev);
static bool dkeyboard_info( parm_link_s *parm, device_s *dev);
static bool dkeyboard_stat( parm_link_s *parm, device_s *dev);
static bool dkeyboard_gen( parm_link_s *parm, device_s *dev);

cmd_s keyboard_cmds[] =
{
	{ "init", (cmd_f)dkeyboard_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "keyboard name" NEXT
		REQ INT "register address" NEXT
		REQ INT "interrupt number" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Displays this help text",
		"Displays this help text",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)dkeyboard_info,
		DEFAULT,
		DEFAULT,
		"Displays keyboard state and configuration",
		"Displays keyboard state and configuration",
		NOCMD},
	{ "stat", (cmd_f)dkeyboard_stat,
		DEFAULT,
		DEFAULT,
		"Displays keyboard statistic",
		"Displays keyboard statistic",
		NOCMD},
	{ "gen", (cmd_f)dkeyboard_gen,
		DEFAULT,
		DEFAULT,
		"Generates a key press with specified code",
		"Generates a key press with specified code",
		REQ VAR "key code" END},
	LAST_CMD
};

const char id_keyboard[] = "dkeyboard";

static void keyboard_done( device_s *d);
static void keyboard_step( device_s *d);
static void keyboard_read( device_s *d, uint32_t addr, uint32_t *val);

device_type_s DKeyboard =
{
	/* device type */
	id_keyboard,
	
	/* brief description*/
	"Keyboard simulation",
	
	/* full description */
	"Device reads key codes from the specified input and sends them to "
		"the system via the interrrupt assert and a memory-mapped "
		"register containing a key code.",
	
	/* functions */
	keyboard_done,	/* done */
	keyboard_step,	/* step */
	keyboard_read,	/* read */
	NULL,		/* write */

	/* commands */
	keyboard_cmds
};


struct keyboard_data_s
{
	uint32_t addr;
	int intno;
	int kcounter;
	char incomming;
	
	bool ig;
	u_int64_t intrcount, keycount, overrun;
	
	int par;
};
typedef struct keyboard_data_s keyboard_data_s;


static void
gen_key( device_s *dev, char k)

{
	keyboard_data_s *kd = dev->data;

	kd->incomming = k;
	kd->keycount++;
			
	if (!kd->ig)
	{
		kd->ig = true;
		kd->intrcount++;
		dcpu_interrupt_up( -1, kd->intno);
	}
	else
		kd->overrun++;

}


/*
 *
 * Commands
 */


/** Init command implementation
 */
static bool
dkeyboard_init( parm_link_s *parm, device_s *dev)

{
	keyboard_data_s *kd;
	
	/* alloc structure */
	kd = malloc( sizeof( *kd));
	if (!kd)
	{
		dprintf( "%s\n", txt_pub[ 5]);
		return false;
	}
	else
		dev->data = kd;
	
	/* initialization */
	parm_next( &parm);
	kd->addr = parm_next_int( &parm);
	kd->intno = parm_next_int( &parm);
	kd->kcounter = 0;
	kd->par = 0;
	
	kd->ig = false;
	kd->intrcount = 0;
	kd->keycount = 0;
	kd->overrun = 0;

	/* checks */
	if (kd->addr & 3)
	{
		dprintf( "Keyboard address must be on 4-byte aligned.\n");
		return false;
	}
	if (kd->intno > 6)
	{
		dprintf( "Interrupt number must be within 0..6.\n");
		return false;
	}

	return true;
}


/** Info command implementation
 */
static bool
dkeyboard_info( parm_link_s *parm, device_s *dev)

{
	keyboard_data_s *kd = dev->data;
	
	dprintf_btag( INFO_SPC, "address:0x%08x " TBRK "intno:%d " TBRK
			"regs(key:0x%02x " TBRK " ig:%d)\n",
			kd->addr, kd->intno, kd->incomming, kd->ig);
	
	return true;
}


/** Stat command implementation
 */
static bool
dkeyboard_stat( parm_link_s *parm, device_s *dev)

{
	keyboard_data_s *kd = dev->data;
	
	dprintf_btag( INFO_SPC, "intrc:%lld " TBRK "keycount:%lld " TBRK
			"overrun:%lld\n",
			kd->intrcount, kd->keycount, kd->overrun);
	
	return true;
}


/** Gen command implementation
 */
static bool
dkeyboard_gen( parm_link_s *parm, device_s *dev)

{
	unsigned char c;

	if (parm->token.ttype == tt_str)
	{
		c = parm->token.tval.s[ 0];

		if (!c || parm->token.tval.s[ 1])
		{
			dprintf( "Invalid key (must be exactly one character).\n");
			return false;
		}
	}
	else /* tt_int */;
	{
		c = parm->token.tval.i;

		if (parm->token.tval.i > 255)
		{
			dprintf( "Invalid key (must be within 0..255).\n");
			return false;
		}
	}

	gen_key( dev, c);
	
	return true;
}


/*
 *
 * Implicit commands
 */


static void
keyboard_done( device_s *d)

{
	XFREE( d->name);
	XFREE( d->data);
}
	

static void
keyboard_read( device_s *d, uint32_t addr, uint32_t *val)
	
{
	keyboard_data_s *kd = d->data;
	
	if (addr == kd->addr)
	{
		*val = kd->incomming;
		if (kd->ig)
		{
			kd->ig = false;
			dcpu_interrupt_down( -1, kd->intno);
		}
	}
}


static void
keyboard_step( device_s *dev)

{
	keyboard_data_s *kd = dev->data;

	if (kd->kcounter++ >= 4096)
	{
		/* test for a new char */
		fd_set rfds;
		struct timeval tv;
		int retval;
		char buf;

		kd->kcounter = 0;

		/* watch stdin */
		FD_ZERO( &rfds);
		FD_SET( 0, &rfds);

		tv.tv_sec = 0; tv.tv_usec = 0;

		retval = select( 1, &rfds, NULL, NULL, &tv);

		if (retval == 1)
		{
			read( 0, &buf, 1);
			gen_key( dev, buf);
		}
	}
}
