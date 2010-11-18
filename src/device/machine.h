/*
 * Copyright (c) 2000-2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MACHINE_H_
#define MACHINE_H_

#include "device.h"
#include "../cpu/r4000.h"
#include "../list.h"
#include "../main.h"
#include "../parser.h"
#include "../endian.h"

#define DEFAULT_MEMORY_VALUE  0xffffffffU

/*
 * Memory area types
 */

typedef enum {
	MEMT_NONE = 0,  /**< Uninitialized */
	MEMT_MEM  = 1,  /**< Generic */
	MEMT_FMAP = 2   /**< File mapped */
} physmem_type_t;

typedef struct {
	item_t item;
	
	/* Memory area type */
	physmem_type_t type;
	bool writable;
	
	/* Basic specification (position and size) */
	ptr36_t start;
	len36_t size;
	
	/* Memory content */
	unsigned char *data;
} physmem_area_t;

typedef struct {
	item_t item;
	cpu_t *cpu;
} sc_item_t;

/** Common variables */
extern bool totrace;
extern bool tohalt;
extern bool tobreak;
extern bool reenter;
extern bool version;

extern list_t physmem_areas;
extern char *config_file;

/** Debug features */
extern char **cp0name;
extern char **cp1name;
extern char **cp2name;
extern char **cp3name;
extern bool change;
extern bool interactive;
extern bool errors;
extern bool script_stat;

extern bool remote_gdb;
extern unsigned int remote_gdb_port;
extern bool remote_gdb_conn;
extern bool remote_gdb_listen;
extern bool remote_gdb_step;

extern uint64_t stepping;
extern list_t sc_list;

extern void input_back(void);

/*
 * Basic machine functions
 */

extern void init_machine(void);
extern void done_machine(void);
extern void go_machine(void);
extern void machine_step(void);

/** Liked Local and Store Conditional control */
extern void register_sc(cpu_t *cpu);
extern void unregister_sc(cpu_t *cpu);

/** Memory access */
extern bool physmem_write(cpu_t *cpu, ptr36_t addr, uint64_t val,
    wsize_t size, bool protected_write);
extern uint64_t physmem_read(cpu_t *cpu, ptr36_t addr, wsize_t size,
    bool protected_read);

#endif
