/*
 * Copyright (c) 2003 Viliam Holub
 * Copyright (c) 2010 Tomas Martinec
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

#include "../list.h"

/** gdb signal numbers */
typedef enum {
    GDB_EVENT_NO_EXCEPTION = 0,
    GDB_EVENT_SYSCALL = 7,
    GDB_EVENT_PAGE_FAULT = 11,
    GDB_EVENT_READ_ONLY_EXC = 7,
    GDB_EVENT_BUS_ERROR = 7,
    GDB_EVENT_ADDRESS_ERROR = 11,
    GDB_EVENT_OVERFLOW = 16,
    GDB_EVENT_ILLEGAL_INSTR = 4,
    GDB_EVENT_BREAKPOINT = 5,
    GDB_EVENT_SOFTWARE_GENERATED = 7
} gdb_event_t;

extern bool gdb_remote_init(void);
extern void gdb_session(void);
extern void gdb_handle_event(gdb_event_t event);

#endif
