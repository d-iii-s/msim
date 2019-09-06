/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "../config.h"
#include "list.h"

#define MAX_CPUS   32
#define MAX_INTRS  6

/** Exception types */
typedef enum {
	excInt   = 0,
	excMod   = 1,
	excTLBL  = 2,
	excTLBS  = 3,
	excAdEL  = 4,
	excAdES  = 5,
	excIBE   = 6,
	excDBE   = 7,
	excSys   = 8,
	excBp    = 9,
	excRI    = 10,
	excCpU   = 11,
	excOv    = 12,
	excTr    = 13,
	excVCEI  = 14,
	excFPE   = 15,
	excWATCH = 23,
	excVCED  = 31,
	
	/* Special exception types */
	excTLBR  = 64,
	excTLBLR = 65,
	excTLBSR = 66,
	
	/* For internal use */
	excNone = 128,
	excJump = 129,
	excAddrError = 130,
	excTLB = 131,
	excReset = 132
} exc_t;

/** Physical frame number type */
typedef uint32_t pfn_t;

/** Address and length types */
typedef uint64_t len36_t;
typedef uint64_t len64_t;
typedef uint64_t ptr36_t;

typedef union {
	uint64_t ptr;
#ifdef WORDS_BIGENDIAN
	struct {
		uint32_t hi;
		uint32_t lo;
	};
#else
	struct {
		uint32_t lo;
		uint32_t hi;
	};
#endif
} __attribute__((packed)) ptr64_t;

typedef union {
	uint64_t val;
#ifdef WORDS_BIGENDIAN
	struct {
		uint32_t hi;
		uint32_t lo;
	};
#else
	struct {
		uint32_t lo;
		uint32_t hi;
	};
#endif
} __attribute__((packed)) reg64_t;

/** Debugging register names */
extern char **regname;
extern char **cp0name;
extern char **cp1name;
extern char **cp2name;
extern char **cp3name;

/** Configuration file name */
extern char *config_file;

/** Remote GDB debugging */
extern bool remote_gdb;
extern unsigned int remote_gdb_port;
extern bool remote_gdb_conn;
extern bool remote_gdb_listen;
extern bool remote_gdb_step;

/** General simulator behaviour */
extern bool machine_nondet;
extern bool machine_trace;
extern bool machine_halt;
extern bool machine_break;
extern bool machine_interactive;
extern bool machine_newline;
extern bool machine_undefined;
extern bool machine_specific_instructions;
extern uint64_t stepping;

#endif
