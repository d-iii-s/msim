/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Exception related constants
 *
 */

#ifndef RISCV_RV_EXCEPTION_H_
#define RISCV_RV_EXCEPTION_H_

#include <stdint.h>

#if XLEN == 32
// MSb that determines if the exception is an interrupt
#define RV_INTERRUPT_EXC_BITS UINT32_C(0x80000000)
#define RV_EXCEPTION_EXC_BITS UINT32_C(0)
#define RV_EXCEPTION_MASK(exc) (1U << ((exc) & ~RV_INTERRUPT_EXC_BITS))
#define RV_INTERRUPT_NO(interrupt) ((interrupt) & ~RV_INTERRUPT_EXC_BITS)
#else
// MSb that determines if the exception is an interrupt
#define RV_INTERRUPT_EXC_BITS UINT64_C(1UL << 63)
#define RV_EXCEPTION_EXC_BITS UINT64_C(0)
#define RV_EXCEPTION_MASK(exc) (UINT64_C(1) << ((exc) & 0x3F))
#define RV_INTERRUPT_NO(interrupt) ((interrupt) & ~RV_INTERRUPT_EXC_BITS)
#endif

/**
 * RISC-V exception codes
 *
 * Includes Interrupt exception codes
 */
typedef enum rv_exc {
    rv_exc_supervisor_software_interrupt = (RV_INTERRUPT_EXC_BITS) | 1,
    rv_exc_machine_software_interrupt = (RV_INTERRUPT_EXC_BITS) | 3,
    rv_exc_supervisor_timer_interrupt = (RV_INTERRUPT_EXC_BITS) | 5,
    rv_exc_machine_timer_interrupt = RV_INTERRUPT_EXC_BITS | 7,
    rv_exc_supervisor_external_interrupt = RV_INTERRUPT_EXC_BITS | 9,
    rv_exc_machine_external_interrupt = RV_INTERRUPT_EXC_BITS | 11,
    rv_exc_instruction_address_misaligned = RV_EXCEPTION_EXC_BITS | 0,
    rv_exc_instruction_access_fault = RV_EXCEPTION_EXC_BITS | 1,
    rv_exc_illegal_instruction = RV_EXCEPTION_EXC_BITS | 2,
    rv_exc_breakpoint = RV_EXCEPTION_EXC_BITS | 3,
    rv_exc_load_address_misaligned = RV_EXCEPTION_EXC_BITS | 4,
    rv_exc_load_access_fault = RV_EXCEPTION_EXC_BITS | 5,
    rv_exc_store_amo_address_misaligned = RV_EXCEPTION_EXC_BITS | 6,
    rv_exc_store_amo_access_fault = RV_EXCEPTION_EXC_BITS | 7,
    rv_exc_umode_environment_call = RV_EXCEPTION_EXC_BITS | 8,
    rv_exc_smode_environment_call = RV_EXCEPTION_EXC_BITS | 9,
    rv_exc_mmode_environment_call = RV_EXCEPTION_EXC_BITS | 11,
    rv_exc_instruction_page_fault = RV_EXCEPTION_EXC_BITS | 12,
    rv_exc_load_page_fault = RV_EXCEPTION_EXC_BITS | 13,
    rv_exc_store_amo_page_fault = RV_EXCEPTION_EXC_BITS | 15,
    rv_exc_none = RV_EXCEPTION_EXC_BITS | 24 /** Custom exception code, with the meaning that no exception has been raised */
} rv_exc_t;

/** Bitmask that has a bit set on every position that symbolizes an existing exception */
#define RV_EXCEPTIONS_MASK ( \
        RV_EXCEPTION_MASK(rv_exc_instruction_address_misaligned) | RV_EXCEPTION_MASK(rv_exc_instruction_access_fault) | RV_EXCEPTION_MASK(rv_exc_illegal_instruction) | RV_EXCEPTION_MASK(rv_exc_breakpoint) | RV_EXCEPTION_MASK(rv_exc_load_address_misaligned) | RV_EXCEPTION_MASK(rv_exc_load_access_fault) | RV_EXCEPTION_MASK(rv_exc_store_amo_address_misaligned) | RV_EXCEPTION_MASK(rv_exc_store_amo_access_fault) | RV_EXCEPTION_MASK(rv_exc_umode_environment_call) | RV_EXCEPTION_MASK(rv_exc_smode_environment_call) | RV_EXCEPTION_MASK(rv_exc_mmode_environment_call) | RV_EXCEPTION_MASK(rv_exc_instruction_page_fault) | RV_EXCEPTION_MASK(rv_exc_load_page_fault) | RV_EXCEPTION_MASK(rv_exc_store_amo_page_fault))
/** Bitmask that has a bit set on every position that symbolizes an existing interrupt */
#define RV_INTERRUPTS_MASK ( \
        RV_EXCEPTION_MASK(rv_exc_supervisor_software_interrupt) | RV_EXCEPTION_MASK(rv_exc_machine_software_interrupt) | RV_EXCEPTION_MASK(rv_exc_supervisor_external_interrupt) | RV_EXCEPTION_MASK(rv_exc_machine_external_interrupt) | RV_EXCEPTION_MASK(rv_exc_supervisor_timer_interrupt) | RV_EXCEPTION_MASK(rv_exc_machine_timer_interrupt))

/**
 * Privilege modes
 */
typedef enum rv_priv_mode {
    rv_umode = 0b00,
    rv_smode = 0b01,
    rv_rmode = 0b10, // RESERVED
    rv_mmode = 0b11
} rv_priv_mode_t;

#endif
