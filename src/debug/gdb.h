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

#include "../mtypes.h"
#include "../list.h"

#define GDB_MAXIMUM_BAD_CHECKSUMS_COUNT  10

#define GDB_NOT_SUPPORTED_STRING                 ""
#define GDB_ERROR_STRING_BAD_MEMORY_COMMAND      "E00"
#define GDB_ERROR_STRING_MEMORY_WRITE_FAIL       "E01"
#define GDB_ERROR_STRING_MEMORY_READ_FAIL        "E02"
#define GDB_ERROR_STRING_UNKNOWN_QUERY           "E03"
#define GDB_ERROR_STRING_BAD_BREAKPOINT_COMMAND  "E04"
#define GDB_ERROR_STRING_REGISTERS_WRITE_FAIL    "E05"

#define GDB_REGISTER_NUMBER_STATUS         32
#define GDB_REGISTER_NUMBER_MULTIPLY_LOW   33
#define GDB_REGISTER_NUMBER_MULTIPLY_HIGH  34
#define GDB_REGISTER_NUMBER_BAD            35
#define GDB_REGISTER_NUMBER_CAUSE          36
#define GDB_REGISTER_NUMBER_PC             37

/** Signal numbers of gdb */
typedef enum {
	GDB_EVENT_NO_EXCEPTION       = 0,
	GDB_EVENT_SYSCALL            = 7,
	GDB_EVENT_PAGE_FAULT         = 11,
	GDB_EVENT_READ_ONLY_EXC      = 7,
	GDB_EVENT_BUS_ERROR          = 7,
	GDB_EVENT_ADDRESS_ERROR      = 11,
	GDB_EVENT_OVERFLOW           = 16,
	GDB_EVENT_ILLEGAL_INSTR      = 4,
	GDB_EVENT_BREAKPOINT         = 5,
	GDB_EVENT_SOFTWARE_GENERATED = 7
} gdb_events_t;

extern bool gdb_remote_init(void);
extern void gdb_session(void);
extern void gdb_handle_event(gdb_events_t event);

#endif /* GDB_H_ */
