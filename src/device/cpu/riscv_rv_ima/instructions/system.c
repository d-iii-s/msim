/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * RISC-V System instructions
 *
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#include <stdint.h>

#include "../../../../assert.h"
#include "../../../../fault.h"
#include "../../../../input.h"
#include "../csr.h"
#include "../exception.h"
#include "../instr.h"

rv_exc_t rv_read_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t *value, bool noisy);
rv_exc_t rv_read_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t *value, bool fetch, bool noisy);
rv_exc_t rv_write_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t value, bool noisy);
rv_exc_t rv_write_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t value, bool noisy);
rv_exc_t rv_write_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t value, bool noisy);
rv_exc_t rv_write_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t value, bool noisy);
rv_exc_t rv_convert_addr(rv_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);

static rv_exc_t rv_break_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    if (input_is_terminal() || machine_allow_interactive_without_tty) {
        alert("EBREAK: breakpoint reached, entering interactive mode");
        machine_interactive = true;
    } else {
        alert("EBREAK: Machine halt when no tty available.");
        machine_halt = true;
    }

    return rv_exc_none;
}

static rv_exc_t rv_halt_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    alert("EHALT: Machine halt");

    machine_halt = true;
    return rv_exc_none;
}

static rv_exc_t rv_trace_set_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("ETRACES: Trace Set");
    machine_trace = true;
    return rv_exc_none;
}

static rv_exc_t rv_trace_reset_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("ETRACES: Trace Reset");
    machine_trace = false;
    return rv_exc_none;
}

static rv_exc_t rv_call_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    switch (cpu->priv_mode) {
    case rv_umode:
        return rv_exc_umode_environment_call;
    case rv_smode:
        return rv_exc_smode_environment_call;
    case rv_mmode:
        return rv_exc_mmode_environment_call;
    default:
        return rv_exc_illegal_instruction;
    }
}

static rv_exc_t rv_sret_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    if (rv_csr_mstatus_tsr(cpu)) {
        return rv_exc_illegal_instruction;
    }
    if (cpu->priv_mode < rv_smode) {
        return rv_exc_illegal_instruction;
    }

    rv_priv_mode_t spp_priv = rv_csr_sstatus_spp(cpu);

    // SIE = SPIE
    {
        if (rv_csr_sstatus_spie(cpu)) {
            cpu->csr.mstatus |= rv_csr_sstatus_sie_mask;
        } else {
            cpu->csr.mstatus &= ~rv_csr_sstatus_sie_mask;
        }
    }
    // priv = SPP
    {
        cpu->priv_mode = spp_priv;
    }
    // SPIE = 1
    {
        cpu->csr.mstatus |= rv_csr_sstatus_spie_mask;
    }
    // SPP = U
    {
        cpu->csr.mstatus &= ~rv_csr_sstatus_spp_mask;
        // SPP = 00 (can be skipped, sice U mode is 0)
    }
    // if SPP != M: MPRV = 0
    {
        // This must always happen, because sret can return only to U or S mode
        cpu->csr.mstatus &= ~rv_csr_mstatus_mprv_mask;
    }

    cpu->pc_next = cpu->csr.sepc;
    return rv_exc_none;
}

static rv_exc_t rv_mret_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    if (cpu->priv_mode < rv_mmode) {
        return rv_exc_illegal_instruction;
    }

    rv_priv_mode_t mpp_priv = rv_csr_mstatus_mpp(cpu);

    // MIE = MPIE
    {
        if (rv_csr_mstatus_mpie(cpu)) {
            cpu->csr.mstatus |= rv_csr_mstatus_mie_mask;
        } else {
            cpu->csr.mstatus &= ~rv_csr_mstatus_mie_mask;
        }
    }
    // priv = MPP
    {
        cpu->priv_mode = mpp_priv;
    }
    // MPIE = 1
    {
        cpu->csr.mstatus |= rv_csr_mstatus_mpie_mask;
    }
    // MPP = U
    {
        cpu->csr.mstatus &= ~rv_csr_mstatus_mpp_mask;
        // MPP = 00 (can be skipped, sice U mode is 00)
    }
    // if MPP != M: MPRV = 0
    {
        if (mpp_priv != rv_mmode) {
            cpu->csr.mstatus &= ~rv_csr_mstatus_mprv_mask;
        }
    }

    cpu->pc_next = cpu->csr.mepc;
    return rv_exc_none;
}

static rv_exc_t rv_wfi_instr(rv_cpu_t *cpu, rv_instr_t instr)
{

    if (rv_csr_mstatus_tw(cpu) && cpu->priv_mode != rv_mmode) {
        return rv_exc_illegal_instruction;
    }

    if (cpu->priv_mode == rv_umode) {
        return rv_exc_illegal_instruction;
    }

    cpu->stdby = true;
    return rv_exc_none;
}

// Note: csrrw reads with rd = x0 shall not read the CSR and shall not have any side-efects based on the read
//       similarly, csrrs and csrrc writes with rs1 = x0 (or uimm = 0) shall not write anything

static rv_exc_t rv_csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = cpu->regs[instr.i.rs1];
    uxlen_t *rd = (uxlen_t *) (&cpu->regs[instr.i.rd]);
    bool read = instr.i.rd != 0;

    return rv_csr_rw((void *) cpu, csr, val, rd, read);
}

static rv_exc_t rv_csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = cpu->regs[instr.i.rs1];
    uxlen_t *rd = (uxlen_t *) &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rs((void *) cpu, csr, val, rd, write);
}

static rv_exc_t rv_csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = cpu->regs[instr.i.rs1];
    uxlen_t *rd = (uxlen_t *) &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rc((void *) cpu, csr, val, rd, write);
}

static rv_exc_t rv_csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = instr.i.rs1; // Zero extended
    uxlen_t *rd = (uxlen_t *) &cpu->regs[instr.i.rd];
    bool read = instr.i.rd != 0;

    return rv_csr_rw((void *) cpu, csr, val, rd, read);
}

static rv_exc_t rv_csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = instr.i.rs1; // Zero extended
    uxlen_t *rd = (uxlen_t *) &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rs((void *) cpu, csr, val, rd, write);
}

static rv_exc_t rv_csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    int csr = ((uint32_t) instr.i.imm) & 0xFFF;
    uxlen_t val = instr.i.rs1; // Zero extended
    uxlen_t *rd = (uxlen_t *) &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rc((void *) cpu, csr, val, rd, write);
}
