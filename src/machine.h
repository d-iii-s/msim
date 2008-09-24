/*
 * machine.h
 * Copyright (c) 2000-2003 Viliam Holub
*/


#ifndef _MACHINE_H_
#define _MACHINE_H_


#include "mtypes.h"
#include "mcons.h"
#include "processor.h"
#include "parser.h"
#include "endi.h"
#include "device/device.h"


enum TMemoryType_enum { mtRWM, mtROM, mtEXC };

typedef struct mem_element_s mem_element_s;
struct mem_element_s
{
	/* memory region type */
	bool writ;

	/* basic specification - position and size */
	uint32_t start;
	uint32_t size;
	
	/* block of memory */
	unsigned char *mem;

	/* next element in the list */
	mem_element_s *next;
};

typedef struct LLList_s LLList_s;
struct LLList_s
{
	processor_s *p;
	LLList_s *next;
};


/* common variables */
extern bool totrace;
extern bool tohalt;
extern int procno;

extern char *config_file;

/* debug features */
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
extern processor_s *focus;
extern mem_element_s *memlist;
extern LLList_s *ll_list;

extern bool tobreak;  /* for readline*/
extern bool reenter; // for readline

bool reenter;

void input_back( void);

/* basic machine functions */
void init_machine( void);
void done_machine( void);
void go_machine( void);
void machine_step( void);

/* debug mode */
void InteractiveControl();
	
/* debug output */
void dprint( const char *s);
	
/* ll and sc control */
void RegisterLL();
void UnregisterLL();
	
/* access memory */
void mem_write( uint32_t addr, uint32_t val, int size);
uint32_t mem_read( uint32_t addr);

/* memory control */
void mem_link( mem_element_s *e);
void mem_unlink( mem_element_s *e);

#endif /* _MACHINE_H_ */
