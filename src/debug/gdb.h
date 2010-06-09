/*
 * Copyright (c) 2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Remote debugging
 *
 */

#ifndef GDB_H_
#define GDB_H_

#include <stdbool.h>

#include "../mtypes.h"
#include "../list.h"

typedef struct {
	item_t item;
	ptr_t pc;
} debug_breakpoint_t;

typedef enum {
	GDB_EVENT_NO_EXCEPTION = 0,
	GDB_EVENT_SYSCALL,
	GDB_EVENT_PAGE_FAULT,
	GDB_EVENT_READ_ONLY_EXC,
	GDB_EVENT_BUS_ERROR,
	GDB_EVENT_ADDRESS_ERROR,
	GDB_EVENT_OVERFLOW,
	GDB_EVENT_ILLEGAL_INSTR,
	GDB_EVENT_BREAKPOINT
} gdb_events_t;

extern list_t debugger_breakpoints;

extern bool gdb_remote_init(void);
extern void gdb_session(void);
extern void gdb_handle_event(gdb_events_t event);

#endif /* GDB_H_ */
