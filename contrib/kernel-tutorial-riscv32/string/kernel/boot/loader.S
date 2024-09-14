// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * System bootstrap code.
 *
 * When RV32IMA is started, it starts executing code at physical address
 * 0xF0000000 in M-mode.
 *
 * This code prepares the environment and jumps to the kernel code.
 */

/** Where the entrypoint to the kernel lies */
#define KERNEL_ENTRYPOINT 0x80001000

.section .loader

/*
 * The execution of the simulator starts here in M-mode.
 *
 * We only jump to our kernel entry point where the setup is done.
 */
__start:
    /* Jump to known kernel entrypoint */
    li t0, KERNEL_ENTRYPOINT
    jr t0
