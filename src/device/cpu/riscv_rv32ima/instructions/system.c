/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * RISC-V System instructions
 *
 */

#include <stdint.h>
#include "system.h"
#include "../../../../input.h"
#include "../../../../main.h"
#include "../../../../assert.h"
#include "../csr.h"
#include "../tlb.h"
#include "../debug.h"

rv_exc_t rv_break_instr(rv_cpu_t *cpu, rv_instr_t instr){
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

rv_exc_t rv_halt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    alert("EHALT: Machine halt");

    machine_halt = true;
    return rv_exc_none;
}

rv_exc_t rv_dump_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("EDUMP: Dumping general registers");
    rv_reg_dump(cpu);
    return rv_exc_none;
}


extern rv_exc_t rv_trace_set_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("ETRACES: Trace Set");
    machine_trace = true;
    return rv_exc_none;
}

extern rv_exc_t rv_trace_reset_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("ETRACES: Trace Reset");
    machine_trace = false;
    return rv_exc_none;
}

extern rv_exc_t rv_csr_rd_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("ECSRRD: Dump CSR");
    uint32_t csr = cpu->regs[instr.i.rd] & 0xFFF;

    if(csr >= 0x1000){
        alert("Wrong CSR number!");
    }
    else {
        rv_csr_dump(cpu, csr);
    }
    return rv_exc_none;
}

rv_exc_t rv_call_instr(rv_cpu_t *cpu, rv_instr_t instr){
    switch(cpu->priv_mode){
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

rv_exc_t rv_sret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    if(rv_csr_mstatus_tsr(cpu)) return rv_exc_illegal_instruction;
    if(cpu->priv_mode < rv_smode) return rv_exc_illegal_instruction;

    rv_priv_mode_t spp_priv = rv_csr_sstatus_spp(cpu);

    // SIE = SPIE
    {
        if(rv_csr_sstatus_spie(cpu)){
            cpu->csr.mstatus |= rv_csr_sstatus_sie_mask;
        }
        else{
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
        //SPP = 00 (can be skipped, sice U mode is 0)
    }
    // if SPP != M: MPRV = 0
    {
        // This must always happen, because sret can return only to U or S mode
        cpu->csr.mstatus &= ~rv_csr_mstatus_mprv_mask;
    }

    cpu->pc_next = cpu->csr.sepc;
    return rv_exc_none;
}

rv_exc_t rv_mret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    if(cpu->priv_mode < rv_mmode) return rv_exc_illegal_instruction;

    rv_priv_mode_t mpp_priv = rv_csr_mstatus_mpp(cpu);

    // MIE = MPIE
    {
        if(rv_csr_mstatus_mpie(cpu)){
            cpu->csr.mstatus |= rv_csr_mstatus_mie_mask;
        }
        else{
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
        //MPP = 00 (can be skipped, sice U mode is 00)
    }
    // if MPP != M: MPRV = 0
    {   if(mpp_priv != rv_mmode){
            cpu->csr.mstatus &= ~rv_csr_mstatus_mprv_mask;
        }
    }

    cpu->pc_next = cpu->csr.mepc;
    return rv_exc_none;
}

rv_exc_t rv_wfi_instr(rv_cpu_t *cpu, rv_instr_t instr){

    if(rv_csr_mstatus_tw(cpu) && cpu->priv_mode != rv_mmode){
        return rv_exc_illegal_instruction;
    }

    if(cpu->priv_mode == rv_umode){
        return rv_exc_illegal_instruction;
    }

    cpu->stdby = true;
    return rv_exc_none;
}

// Note: csrrw reads with rd = x0 shall not read the CSR and shall not have any side-efects based on the read
//       similarly, csrrs and csrrc writes with rs1 = x0 (or uimm = 0) shall not write anything

rv_exc_t rv_csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool read = instr.i.rd != 0;

    return rv_csr_rw(cpu, csr, val, rd, read);
}

rv_exc_t rv_csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rs(cpu, csr, val, rd, write);
}

rv_exc_t rv_csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rc(cpu, csr, val, rd, write);
}

rv_exc_t rv_csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1; // Zero extended
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool read = instr.i.rd != 0;

    return rv_csr_rw(cpu, csr, val, rd, read);
}

rv_exc_t rv_csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1; // Zero extended
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rs(cpu, csr, val, rd, write);
}

rv_exc_t rv_csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1; // Zero extended
    uint32_t* rd = &cpu->regs[instr.i.rd];
    bool write = instr.i.rs1 != 0;

    return rv_csr_rc(cpu, csr, val, rd, write);
}

rv_exc_t rv_sfence_instr(rv_cpu_t *cpu, rv_instr_t instr) {
    if(rv_csr_mstatus_tvm(cpu) || cpu->priv_mode < rv_smode){
        return rv_exc_illegal_instruction;
    }

    if(instr.r.rs1 == 0){
        if(instr.r.rs2 == 0){
            // rs1 == x0 && rs2 == x0
            rv_tlb_flush(&cpu->tlb);
        }
        else {
            // rs1 == x0 && rs2 != x0
            rv_tlb_flush_by_asid(&cpu->tlb, cpu->regs[instr.r.rs2] & rv_asid_mask);
        }
    }
    else {
        if(instr.r.rs2 == 0){
            // rs1 != x0 && rs2 == x0
            rv_tlb_flush_by_addr(&cpu->tlb, cpu->regs[instr.r.rs1]);
        }
        else {
            // rs1 != x0 && rs2 != x0
            rv_tlb_flush_by_asid_and_addr(&cpu->tlb, cpu->regs[instr.r.rs2] & rv_asid_mask, cpu->regs[instr.r.rs1]);
        }
    }
    return rv_exc_none;
}