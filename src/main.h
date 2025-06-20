/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdint.h>

#include "../config.h"
#include "list.h"

#define MAX_CPUS 32
#define MAX_INTRS 11

/** Physical frame number type */
typedef uint32_t pfn_t;

/** Address and length types */
typedef uint64_t len36_t;
typedef uint64_t len64_t;
typedef uint64_t ptr36_t;
typedef uint64_t ptr55_t;

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
} __attribute__((packed, aligned(8))) ptr64_t;

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
} __attribute__((packed, aligned(8))) reg64_t;

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
extern bool machine_allow_interactive_without_tty;
extern uint64_t stepping;

#endif
