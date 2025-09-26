// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _DEBUG_H
#define _DEBUG_H

#include <drivers/machine.h>
#include <common.h>

/* Helper macros. */
#define QUOTE_ME_(x) #x
#define QUOTE_ME(x) QUOTE_ME_(x)

/*
 * dprintk works like printk but also adds source code information.
 */
#ifdef KERNEL_DEBUG
#define dprintk(fmt, ...) \
    printk("[DEBUG " __FILE__ ":" QUOTE_ME(__LINE__) " %s()] " fmt, \
            __func__, ##__VA_ARGS__)
#else
#define dprintk(...) ((void)0)
#endif

/*
 * When assertion in kernel fails, we halt the machine.
 */
#define assert(expr) \
    do { \
        if (!(expr)) { \
            puts("Assert failed at " __FILE__ ":" QUOTE_ME(__LINE__) ": " #expr); \
            machine_halt_via_debugger(); \
        } \
    } while (0)

/*
 * Panic functions to halt the machine from unrecoverable errors.
 */

#define panic_if(expr, fmt, ...) \
    do { \
        if ((expr)) { \
            printk("Kernel panic: " fmt "\n", ##__VA_ARGS__); \
            puts("Location: " __FILE__ ":" QUOTE_ME(__LINE__)); \
            puts("Condition: " #expr); \
            machine_halt_via_debugger(); \
        } \
    } while (0)

#define panic(fmt, ...) \
    do { \
        printk("Kernel panic: " fmt "\n", ##__VA_ARGS__); \
        puts("Location: " __FILE__ ":" QUOTE_ME(__LINE__)); \
        machine_halt_via_debugger(); \
    } while (0)

#endif
