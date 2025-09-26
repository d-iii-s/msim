// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <common.h>
#include <ktest.h>
#include <main.h>

/** Halt the execution of the simulator */
static inline void halt(void) {
    __asm__ volatile(".word 0x8C000073");
}

/** This is kernel C-entry point.
 *
 * Kernel jumps here from assembly bootstrap code. Note that
 * this function runs on special stack and does not represent a
 * real thread.
 *
 * When the code is compiled to run kernel test, we execute only
 * that test and terminate.
 */
void kernel_main(void) {
#ifdef KERNEL_TEST
    kernel_test();
#else
    putchar('H');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar(',');
    putchar(' ');
    putchar('W');
    putchar('o');
    putchar('r');
    putchar('l');
    putchar('d');
    putchar('!');
    putchar('\n');
#endif
    halt();
}
