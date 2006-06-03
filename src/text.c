/*
 * text.c
 * text constans
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <errno.h>
#include <string.h>

#include "text.h"


const char *text[] =
{
	"MSIM - Machine Simulator\n",
	"Error: Memory dump allowed only for focused processor.\n"
};


const char *excText[] =
{
// 0
	"Interrupt",
	"TLB modification",
	"TLB (load or instruction fetch)",
	"TLB (store)",
	"Address error (load or instruction fetch)",
	"Address error (store)",
	"Bus error (instruction fetch)",
	"Bus error (data reference: load or store)",
// 8
	"Syscall",
	"Breakpoint",
	"Reserved instruction",
	"Coprocessor Unusable",
	"Arithmetic Overflow",
	"Trap",
	"Virtual Coherency instruction",
	"Floating-Point",
// 16
	"", "", "", "", "", "", "",
	"Reference to WatchHi/WatchLo address",
	"", "", "", "", "", "", "",
	"Virtual Coherency data"
};

const char txt_linehelp[] =
		"[command]           [description]\n"
		"  add dev name ...  add device\n"
		"  s [cnt]           step one/cnt instructions\n"
		"  g [addr]          go to address addr/continue\n"
		"  md addr cnt       dumps memory from addr cnt words\n"
		"  id addr cnt       dumps cnt instructions from addr\n"
		"  dd                dumps all installed devices\n"
		"  msd               memory structure dump\n"
		"  stat              print several statistics\n"
		"  set [arg val]     set variable\n"
		"  h                 help\n"
		"  q                 quit\n";

const char txt_version[] =
		PACKAGE " version " VERSION " Copyright (c) 2000-2006 Viliam Holub\n";
		
const char txt_help[] =
		"  -V, --version            display version info\n"
		"  -c, --config=file_name   configuration file name\n"
		"  -i, --interactive        enter interactive mode\n"
		"  -t, --trace              enter trace mode\n"
		"  --remote-gdb port        enter gdb mode\n";
		
const char hexchar[] = "0123456789abcdef";
