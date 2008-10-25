/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  String constants
 *
 */

#include <errno.h>
#include <string.h>

#include "../config.h"
#include "text.h"
#include "main.h"


const char *exc_text[] = {
	/* 0 */
	"Interrupt",
	"TLB modification",
	"TLB (load or instruction fetch)",
	"TLB (store)",
	"Address error (load or instruction fetch)",
	"Address error (store)",
	"Bus error (instruction fetch)",
	"Bus error (data reference: load or store)",
	/* 8 */
	"Syscall",
	"Breakpoint",
	"Reserved instruction",
	"Coprocessor Unusable",
	"Arithmetic Overflow",
	"Trap",
	"Virtual Coherency instruction",
	"Floating-Point",
	/* 16 */
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Reference to WatchHi/WatchLo address",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Virtual Coherency data"
};


const char txt_version[] =
	PACKAGE " version " VERSION " Copyright (c) 2000-2008 Viliam Holub\n";

		
const char txt_help[] =
	"  -V, --version            display version info\n"
	"  -c, --config=file_name   configuration file name\n"
	"  -i, --interactive        enter interactive mode\n"
	"  -t, --trace              enter trace mode\n"
	"  -g, --remote-gdb=port    enter gdb mode\n";

		
const char hexchar[] = "0123456789abcdef";
