/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Control and Status registers implementation
 *
 */

#include <stdint.h>

#include "../../../assert.h"
#include "../../../utils.h"
#include "cpu.h"
#include "csr.h"
#include "tlb.h"

#define rv_cpu rv64_cpu
#define rv_cpu_t rv64_cpu_t

/** Generic CSR implementation */
#include "../riscv_rv_ima/csr.c"
#include "../riscv_rv_ima/csr.h"
#include "../riscv_rv_ima/types.h"

extern void rv64_init_csr(rv_csr_t *csr, unsigned int procno)
{
    rv_init_csr(csr, procno);
}

extern enum rv_priv_mode rv64_csr_min_priv_mode(csr_num_t csr)
{
    return rv_csr_min_priv_mode(csr);
}

extern enum rv_exc rv64_csr_rw(rv_cpu_t *cpu, csr_num_t csr, uint64_t value, uint64_t *read_target, bool read)
{
    return rv_csr_rw(cpu, csr, value, (uxlen_t *) read_target, read);
}

extern enum rv_exc rv64_csr_rs(rv_cpu_t *cpu, csr_num_t csr, uint64_t value, uint64_t *read_target, bool write)
{
    return rv_csr_rs(cpu, csr, value, (uxlen_t *) read_target, write);
}

extern enum rv_exc rv64_csr_rc(rv_cpu_t *cpu, csr_num_t csr, uint64_t value, uint64_t *read_target, bool write)
{
    return rv_csr_rs(cpu, csr, value, (uxlen_t *) read_target, write);
}

extern void rv64_csr_set_asid_len(rv_cpu_t *cpu, unsigned asid_len)
{
    ASSERT(asid_len <= rv_asid_len);

    cpu->csr.asid_len = asid_len;

    uint64_t zeroing_asid_mask = (rv_asid_mask ^ ((1UL << asid_len) - 1)) << rv_csr_satp_asid_offset;
    cpu->csr.satp &= ~zeroing_asid_mask;

    rv64_tlb_flush(&cpu->tlb);
}
