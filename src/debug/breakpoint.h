/*
 * Copyright (c) 2010 Tomas Martinec
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef BREAKPOINT_H_
#define BREAKPOINT_H_

#include <stdbool.h>
#include "../list.h"
#include "../main.h"

/** Kind of code breakpoints */
typedef enum {
	BREAKPOINT_KIND_DEBUGGER  = 0x01,
	BREAKPOINT_KIND_SIMULATOR = 0x02
} breakpoint_kind_t;

/** Filter for filtering the code breakpoints according to the kind */
typedef enum {
	BREAKPOINT_FILTER_DEBUGGER  = BREAKPOINT_KIND_DEBUGGER,
	BREAKPOINT_FILTER_SIMULATOR = BREAKPOINT_KIND_SIMULATOR,
	BREAKPOINT_FILTER_ANY       = BREAKPOINT_KIND_DEBUGGER | BREAKPOINT_KIND_SIMULATOR
} breakpoint_filter_t;

/** Memory access types */
typedef enum {
	ACCESS_READ  = 0x01,
	ACCESS_WRITE = 0x02
} access_t;

/** Filter for filtering the memory operations according to the access type */
typedef enum {
	ACCESS_FILTER_NONE  = 0,
	ACCESS_FILTER_READ  = ACCESS_READ,
	ACCESS_FILTER_WRITE = ACCESS_WRITE,
	ACCESS_FILTER_ANY   = ACCESS_READ | ACCESS_WRITE
} access_filter_t;

/** Structure for the code breakpoints */
typedef struct {
	item_t item;

	breakpoint_kind_t kind;
	ptr64_t pc;
	uint64_t hits;
} breakpoint_t;

/** Structure for the memory breakpoints */
typedef struct {
	item_t item;

	breakpoint_kind_t kind;
	ptr36_t addr;
	len36_t size;
	uint64_t hits;
	access_filter_t access_flags;
} physmem_breakpoint_t;

/** List of all the memory breakpoints */
extern list_t physmem_breakpoints;

/* Memory breakpoints interface */

extern void physmem_breakpoint_add(ptr36_t address, len36_t size,
    breakpoint_kind_t kind, access_filter_t access_flags);
extern bool physmem_breakpoint_remove(ptr36_t address);
extern void physmem_breakpoint_remove_filtered(breakpoint_filter_t filter);
extern void physmem_breakpoint_hit(physmem_breakpoint_t *breakpoint,
    access_t access_type);
extern void physmem_breakpoint_print_list(void);

/* Code breakpoints interface */

extern breakpoint_t *breakpoint_init(ptr64_t address, breakpoint_kind_t kind);
extern breakpoint_t *breakpoint_find_by_address(list_t breakpoints,
    ptr64_t address, breakpoint_filter_t filter);
extern bool breakpoint_check_for_code_breakpoints(void);

#endif
