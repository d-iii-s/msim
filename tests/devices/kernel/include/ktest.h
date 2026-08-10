// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _KTEST_H
#define _KTEST_H

#include <debug.h>

/*
 * Prefixes for test output, used by the check_output.py script.
 */
#define KTEST_EXPECTED "[EXPECTED]: "
#define KTEST_BLOCK_EXPECTED "[EXPECTED BLOCK]: "
#define KTEST_ACTUAL "[ ACTUAL ]: "

/** Prints the header of a kernel test. */
#define ktest_start(name) \
    puts("== KERNEL TEST " name " ==")

/** Prints a message about passed kernel test. */
#define ktest_passed() \
    puts("\n\nTest finished.\n\n")

/** Prints a message about failed kernel test and halts the CPU. */
#define ktest_failed() \
    do { \
        puts("\n\nTest failed.\n\n"); \
        machine_halt(); \
    } while (0)

/** Prints a message for tester to signal that this test panics the kernel. */
#define ktest_expect_panic() \
    puts("\n\n[ ENDS WITH PANIC ]\n\n")

/** Kernel test assertion.
 *
 * Unlike a normal assertion, this one is always checked and the
 * machine is halted when expr does not evaluate to true.
 */
#define ktest_assert(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            puts("\n\n" __FILE__ ":" QUOTE_ME(__LINE__) ": Kernel test assertion failed: " #expr); \
            printk(__FILE__ ":" QUOTE_ME(__LINE__) ": " fmt "\n", ##__VA_ARGS__); \
            ktest_failed(); \
        } \
    } while (0)

/** Kernel test assertion for bound checking. */
#define ktest_assert_in_range(msg, value, lower, upper) \
    ktest_assert(((value) >= (lower)) && ((value) <= (upper)), \
            "%s: " #value "=%d not in [" #lower "=%d, " #upper "=%d] range", \
            msg, value, lower, upper)

/** Kernel test asserting for error codes. */
#define ktest_assert_errno(value, last_operation_name) \
    ktest_assert((value) == EOK, "%s unexpectedly failed with %d (%s)", \
            last_operation_name, (value), errno_as_str(value))

/** Kernel test assertion to ensure interrupts are enabled.
 *
 * We abuse the existing API and disable interrupts for a short while to
 * check their status. We try to keep the API small thus we do not add
 * new function for merely checking their status (the actual time the
 * interrupts are disabled is few instructions long anyway).
 */
#define ktest_assert_interrupts_enabled() \
    do { \
        bool interrupts_are_enabled_ = interrupts_disable(); \
        ktest_assert(interrupts_are_enabled_, "Interrupts should be enabled here."); \
        interrupts_restore(interrupts_are_enabled_); \
    } while (0)

/** Kernel test assertion to ensure interrupts are disabled.
 *
 * We abuse the existing API and disable interrupts for a short while to
 * check their status. We try to keep the API small thus we do not add
 * new function for merely checking their status (the actual time the
 * interrupts are disabled is few instructions long anyway).
 */
#define ktest_assert_interrupts_disabled() \
    do { \
        bool interrupts_are_enabled_ = interrupts_disable(); \
        ktest_assert(!interrupts_are_enabled_, "Interrupts should be disabled here."); \
        interrupts_restore(interrupts_are_enabled_); \
    } while (0)

/** Kernel test signature.
 *
 * All kernel tests share this signature. This is possible
 * because only one test is compiled at a time.
 */
void kernel_test(void);

#endif
