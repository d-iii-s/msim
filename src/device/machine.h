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


enum TMemoryType_enum {
	mtRWM,
	mtROM,
	mtEXC
};

typedef struct mem_element_s mem_element_s;
struct mem_element_s {
	/* Memory region type */
	bool writ;

	/* Basic specification - position and size */
	uint32_t start;
	uint32_t size;
	
	/* Block of memory */
	unsigned char *mem;

	/* Next element in the list */
	mem_element_s *next;
};


typedef struct llist {
	processor_t *p;
	struct llist *next;
} llist_t;


typedef struct {
	item_t item;
	
	addr_t addr;
	unsigned long long hits;
	bool rd;
	bool wr;
} mem_breakpoint_t;


/**< Memory breakpoints */
extern list_t mem_bps;


/**< Common variables */
extern bool totrace;
extern bool tohalt;
extern int procno;

extern char *config_file;

/**< Debug features */
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
extern bool remote_gdb_step;
extern bool remote_gdb_one_step;

extern bool version;

extern uint32_t stepping;
extern mem_element_s *memlist;
extern llist_t *ll_list;

extern bool tobreak;
extern bool reenter;

extern void input_back(void);


/*
 * Basic machine functions
 */

extern void init_machine(void);
extern void done_machine(void);
extern void go_machine(void);
extern void machine_step(void);

/**< ll and sc control */
extern void register_ll(processor_t *pr);
extern void unregister_ll(processor_t *pr);
	
/**< Memory access */
extern void mem_write(processor_t *pr, uint32_t addr, uint32_t val, int size);
extern uint32_t mem_read(processor_t *pr, uint32_t addr);

/**< Memory control */
extern void mem_link(mem_element_s *e);
extern void mem_unlink(mem_element_s *e);

#endif /* MACHINE_H_ */
