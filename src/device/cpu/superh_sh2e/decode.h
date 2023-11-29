/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_DECODE_H_
#define SUPERH_SH2E_DECODE_H_

#include "cpu.h"
#include "insn.h"

#include <stdint.h>

/** Instruction disassembly. */
struct sh2e_insn_desc;

typedef void (*sh2e_insn_desc_disasm_fn_t)(
    struct sh2e_insn_desc const * desc,
    sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn,
    /* output */ string_t * mnemonics,
    /* output */ string_t * comments
);


/** Instruction descriptor. */
typedef struct sh2e_insn_desc {
    char const * const assembly;
    char const * const abstract;
    /* sh2e_insn_exec_fn_t */ void * const exec;
    sh2e_insn_desc_disasm_fn_t disasm;
    // unsigned const scale; /** Displacement scaling parameter. */
    unsigned const cycles;
    unsigned const branch_cycles; /** Extra cycles (in addition to `cycles`) when a branch is taken. */
    bool const disable_interrupts; /** Ignore interrupts between this and the following instruction. */
    bool const bus_lock; /** Lock bus while executing the instruction. */
} sh2e_insn_desc_t;


/**
 * Instruction data.
 * Intended to allow executing
 * instruction with precomputed pointers to register operands,
 * PC-relative memory addresses, and various offsets.
 * On the other hand, this may require too many memory accesses.
 * Currently unused.
 */
typedef struct sh2e_insn_data {
    union {
        PACKED struct {
            uint32_t * rn;
            uint32_t * rm;
        };

        PACKED struct {
            float32_t * frn;
            float32_t * frm;
        };
    };

    uint32_t offset;
    uint32_t addr;
} sh2e_insn_data_t;


extern sh2e_insn_desc_t const * sh2e_insn_decode(sh2e_insn_t insn);

#endif // SUPERH_SH2E_DECODE_H_
