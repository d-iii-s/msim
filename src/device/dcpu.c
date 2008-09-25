/*
 * dcpu.c
 * R4000 microprocessor (32bit) device
 * Copyright (c) 2003,2004 Viliam Holub
 *
 * the encappsulation of the processor
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../mcons.h"
#include "device.h"
#include "../cpu/processor.h"
#include "../debug/debug.h"
#include "../output.h"
#include "../utils.h"

#include "dcpu.h"


/*
 * Dcpu commands
 */
static bool dcpu_init( parm_link_s *parm, device_s *dev);
static bool dcpu_info( parm_link_s *parm, device_s *dev);
static bool dcpu_stat( parm_link_s *parm, device_s *dev);
static bool dcpu_cp0d( parm_link_s *parm, device_s *dev);
static bool dcpu_tlbd( parm_link_s *parm, device_s *dev);
static bool dcpu_md( parm_link_s *parm, device_s *dev);
static bool dcpu_id( parm_link_s *parm, device_s *dev);
static bool dcpu_rd( parm_link_s *parm, device_s *dev);
static bool dcpu_goto( parm_link_s *parm, device_s *dev);

cmd_s dcpu_cmds[] =
{
	{ "init", (cmd_f)dcpu_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "pname/processor name" END
	},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Displays this help text",
		"Displays this help text",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)dcpu_info,
		DEFAULT,
		DEFAULT,
		"Displays configuration information",
		"Displays configuration information",
		NOCMD},
	{ "stat", (cmd_f)dcpu_stat,
		DEFAULT,
		DEFAULT,
		"Displays processor statistics",
		"Displays processor statistics",
		NOCMD},
	{ "cp0d", (cmd_f)dcpu_cp0d,
		DEFAULT,
		DEFAULT,
		"Dumps contents of the coprocessor 0 register(s)",
		"Dumps contents of the coprocessor 0 register(s)",
		OPT INT "rn/register number" END},
	{ "tlbd", (cmd_f)dcpu_tlbd,
		DEFAULT,
		DEFAULT,
		"Dumps content of TLB",
		"Dumps content of TLB",
		NOCMD},
	{ "md", (cmd_f)dcpu_md,
		DEFAULT,
		DEFAULT,
		"Dumps specified TLB mapped memory block",
		"Dumps specified TLB mapped memory block",
		REQ INT "saddr/starting address" NEXT
		REQ INT "size/size" END},
	{ "id", (cmd_f)dcpu_id,
		DEFAULT,
		DEFAULT,
		"Dumps instructions from specified TLB mapped memory",
		"Dumps instructions from specified TLB mapped memory",
		REQ INT "sa/starting address" NEXT
		REQ INT "cnt/count" END},
	{ "rd", (cmd_f)dcpu_rd,
		DEFAULT,
		DEFAULT,
		"Dumps contents of CPU general registers",
		"Dumps contents of CPU general registers",
		NOCMD},
	{ "goto", (cmd_f)dcpu_goto,
		DEFAULT,
		DEFAULT,
		"Go to address",
		"Go to address",
		REQ INT "na/new address" END},
	LAST_CMD
};

const char id_dcpu[] = "dcpu";

static void dcpu_done( device_s *dev);
static void dcpu_step( device_s *dev);

device_type_s DCPU =
{
	/* type name */
	id_dcpu,

	/* brief description*/
	"MIPS R4000 processor",

	/* full description */
	"Implementation of the MIPS R4000 processor restricted to 32bits "
	"with no fpu.",
	
	/* functions */
	.done  = dcpu_done,	/* done */
	.step  = dcpu_step,	/* step */
	.step4 = NULL, 		/* step4 */
	.read  = NULL,		/* read */
	.write = NULL,		/* write */

	/* commands */
	dcpu_cmds
};

int R4000_cnt = 0;

struct cpu_data_s
{
	int cpuno;
	processor_s *proc;
};
typedef struct cpu_data_s cpu_data_s;

	
/*
 * Forward declarations
 */
static int cpu_get_free_id( void);



/** Inicialization.
 */
static bool
dcpu_init( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd;
	
	cd = XXMALLOC( cpu_data_s);
	dev->data = cd;
	
	cd->proc = XXMALLOC( processor_s);

	cd->cpuno = cpu_get_free_id();
	if (cd->cpuno == -1)
	{
		mprintf( "Maximum CPU count exceeded (31).");
		free( cd);
		return false;
	}
	
	pr = cd->proc;
	processor_init( cd->cpuno);
	
	R4000_cnt++;

	return true;
}


/** Info command implementation.
 */
static bool
dcpu_info( parm_link_s *parm, device_s *dev)

{
	mprintf_btag( INFO_SPC, "type:R4000.32\n");
	return true;
}


/** Stat command implementation.
 */
static bool
dcpu_stat( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	processor_s *p = cd->proc;
	
        mprintf_btag( INFO_SPC, "cycles total:%lld " TBRK
			"in kernel:%lld " TBRK "in user:%lld " TBRK
			"in stdby:%lld " TBRK 
			"tlb refill:%lld " TBRK "invalid: %lld " TBRK
			"modified:%lld " TBRK
			"interrupts 0:%lld 1:%lld 2:%lld 3:%lld 4:%lld 5:%lld"
			" 6:%lld 7:%lld\n",
			(long long)p->k_cycles +p->u_cycles +p->w_cycles,
			(long long)p->k_cycles,
			(long long)p->u_cycles,
			(long long)p->w_cycles,
			(long long)p->tlb_refill,
			(long long)p->tlb_invalid,
			(long long)p->tlb_modified,
			(long long)p->intr[ 0], (long long)p->intr[ 1],
			(long long)p->intr[ 2], (long long)p->intr[ 3],
			(long long)p->intr[ 4], (long long)p->intr[ 5],
			(long long)p->intr[ 6], (long long)p->intr[ 7]);
	
	return true;
}


/** Cp0d command implementation.
 */
static bool
dcpu_cp0d( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	int no=-1;
	
	if (parm->token.ttype == tt_int)
	{
		no = parm->token.tval.i;
		if (no > 31)
		{
			mprintf_btag( INFO_SPC, "Out of range (0..31).");
			return false;
		}
	}
	
	pr = cd->proc;
	cp0_dump(no);
	
	return true;
}


/** Tlbd command implementation.
 */
static bool
dcpu_tlbd( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;

	pr = cd->proc;
	tlb_dump();

	return true;
}


/** Md command implementation.
 */
static bool
dcpu_md( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	int j;
	uint32_t val, addr, siz;
	enum exc res;
	
	pr = cd->proc;
	addr = parm->token.tval.i & ~0x3;
	siz = parm->next->token.tval.i;
	
	for (j=0; siz; siz--, addr+=4, j++)
	{
		if (!(j&0x3))
			mprintf( "  %08x    ", addr);
		
		res = read_proc_mem( addr, 4, &val, false);
		mprintf( "res: %d\n", res);
		
		if (res == excNone)
			mprintf( "%08x  ", val);
		else
			mprintf( "xxxxxxxx  ");
		
		if ((j&0x3) == 3)
			mprintf( "\n");
	}

	if (j)  
		mprintf( "\n");

	return true;
}


/** Id command implementation.
 */
static bool
dcpu_id( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	enum exc res;
	TInstrInfo ii;
	uint32_t addr, siz;
	
	pr = cd->proc;
	addr = parm->token.tval.i & ~0x3;
	siz = parm->next->token.tval.i;
	ii.rt = 0; ii.rd = 0; ii.rs = 0;
	
	for (; siz; siz--, addr+=4)
	{

		res = read_proc_ins( addr, &ii.icode, false);
		
		if (res != excNone)
		{
			ii.icode = 0;
			ii.opcode = opcIllegal;
		}
		else
			decode_instr( &ii);

		iview( addr, &ii, true, 0);
	}

	return true;
}


/** Rd command implementation.
 */
static bool
dcpu_rd( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	
	pr = cd->proc;
	reg_view();
	
	return true;
}


/** Goto command implementation.
 */
static bool
dcpu_goto( parm_link_s *parm, device_s *dev)

{
	cpu_data_s *cd = dev->data;
	uint32_t addr = parm->token.tval.i;
	
	cd->proc->pcreg = addr;
	cd->proc->pcnextreg = addr+4;
	
	return true;
}


/*
 *
 * Implicit commands
 */


/** Done
 */
static void
dcpu_done( device_s *dev)

{
	XFREE( dev->name);
	if (dev->data)
		XFREE( ((cpu_data_s *)dev->data)->proc);
	XFREE( dev->data);
	R4000_cnt--;
}


/** Execute one processor step.
 */
static void
dcpu_step( device_s *dev)

{
	cpu_data_s *cd = dev->data;
	
	pr = cd->proc;
	step();
}



static int
cpu_get_free_id( void)

{
	int c;
	unsigned id_mask = 0;
	device_s *d = 0;

	while (dev_next( &d))
		if (d->type->name == id_dcpu)
			id_mask |= 1 << ((cpu_data_s *)
					d->data)->cpuno;
	
	for (c=0; c<MAXPROC; c++,id_mask>>=1)
		if (!(id_mask & 1))
			return c;

	return -1;
}


processor_s *
cpu_find_no( int no)

{
	device_s *d = 0;

	while (dev_next( &d))
		if (d->type->name == id_dcpu)
			if (((cpu_data_s *) d->data)->cpuno == no)
				return ((cpu_data_s *)
						d->data)->proc;
	
	return 0;
}

void
dcpu_interrupt_up( int cpuno, int no)

{
	processor_s *px;

	if (cpuno == -1)
		cpuno = 0;
	
	px = pr;
	if ((pr = cpu_find_no( cpuno)))
		proc_interrupt_up( no);
	pr = px;
}


void
dcpu_interrupt_down( int cpuno, int no)

{
	processor_s *px;

	if (cpuno == -1)
		cpuno = 0;
	
	px = pr;
	if ((pr = cpu_find_no( cpuno)))
		proc_interrupt_down( no);
	pr = px;
}
