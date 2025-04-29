/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Control and Status registers
 *
 */

#ifndef RISCV_CSR_H_
#define RISCV_CSR_H_

#include <stdbool.h>
#include <stdint.h>

#include "../riscv_rv_ima/csr.h"
#include "../riscv_rv_ima/exception.h"

extern void rv32_init_csr(rv_csr_t *csr, unsigned int procno);

extern enum rv_priv_mode rv32_csr_min_priv_mode(csr_num_t csr);

extern enum rv_exc rv32_csr_rw(rv_cpu_t *cpu, csr_num_t csr, uint32_t value, uint32_t *read_target, bool read);
extern enum rv_exc rv32_csr_rs(rv_cpu_t *cpu, csr_num_t csr, uint32_t value, uint32_t *read_target, bool write);
extern enum rv_exc rv32_csr_rc(rv_cpu_t *cpu, csr_num_t csr, uint32_t value, uint32_t *read_target, bool write);

/** Change the number of active ASID bits (used in the virtual address translation)
 *  This function zeroes-out any bits that will not be active in the ASID field of the SATP CSR
 *  This function fully flushes the TLB
 */
extern void rv32_csr_set_asid_len(rv_cpu_t *cpu, unsigned asid_active_bits);

#endif // RISCV_CSR_H_
