/*
 * Copyright (c) 2000-2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MACHINE_H_
#define MACHINE_H_


#include "../mtypes.h"
#include "../list.h"
#include "../main.h"
#include "../cpu/processor.h"
#include "../parser.h"
#include "../endi.h"
#include "device.h"

#define DEFAULT_MEMORY_VALUE  0xffffffffUL

#define MEM_AREAS  32

/*
 * Memory area types
 */

typedef enum {
	MEMT_NONE = 0,  /**< Uninitialized */
	MEMT_MEM  = 1,  /**< Generic */
	MEMT_FMAP = 2   /**< File mapped */
} mem_type_t;

typedef struct {
	/* Area index */
	size_t index;
	
	/* Memory area type */
	mem_type_t type;
	bool writable;
	
	/* Basic specification (position and size) */
	ptr_t start;
	len_t size;
	
	/* Memory content */
	unsigned char *data;
} mem_area_t;

typedef struct llist {
	processor_t *p;
	struct llist *next;
} llist_t;

/** Common variables */
extern bool totrace;
extern bool tohalt;
extern bool tobreak;
extern bool reenter;
extern bool version;

extern int procno;

extern size_t max_mem_areas;
extern mem_area_t mem_areas[MEM_AREAS];

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
extern int remote_gdb_port;
extern bool remote_gdb_conn;
extern bool remote_gdb_listen;
extern bool remote_gdb_step;

extern uint32_t stepping;
extern llist_t *ll_list;

extern void input_back(void);

/*
 * Basic machine functions
 */

extern void init_machine(void);
extern void done_machine(void);
extern void go_machine(void);
extern void machine_step(void);

/** ll and sc control */
extern void register_ll(processor_t *pr);
extern void unregister_ll(processor_t *pr);

/** Memory access */
extern bool mem_write(processor_t *pr, uint32_t addr, uint32_t val,
    size_t size, bool protected_write);
extern uint32_t mem_read(processor_t *pr, uint32_t addr, size_t size,
    bool protected_read);

#endif /* MACHINE_H_ */
