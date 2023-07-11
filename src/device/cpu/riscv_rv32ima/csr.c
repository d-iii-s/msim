/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Control and Status registers implementation
 *
 */

#include <stdint.h>
#include "csr.h"
#include "cpu.h"
#include "../../../assert.h"
#include "../../../utils.h"

/**
 * Initialize CSRs
 * 
 * Expects the csr parameter to be zero-initialized
 */
void rv_init_csr(csr_t *csr, unsigned int procno){
    
    csr->misa = RV_ISA;
    csr->mvendorid = RV_VENDOR_ID;
    csr->marchid = RV_ARCH_ID;
    csr->mimpid = RV_IMPLEMENTATION_ID;
    csr->mhartid = procno;
    
    csr->mtime = current_timestamp();
    csr->last_tick_time = csr->mtime;
}

#define minimal_privilege(priv, cpu) {if(cpu->priv_mode < priv) return rv_exc_illegal_instruction;}
#define default_csr_functions(csr_name, priv)                                       \
    static rv_exc_t csr_name##_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){\
        minimal_privilege(priv, cpu);                                               \
        *target = cpu->csr.csr_name;                                                \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){ \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name = value;                                                  \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){   \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name |= value;                                                 \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){ \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name &= ~value;                                                \
        return rv_exc_none;                                                         \
    }


static rv_exc_t invalid_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}
static rv_exc_t invalid_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

#define is_counter_enabled_m(cpu, counter) (cpu->csr.mcounteren & (1<<counter))
#define is_counter_enabled_s(cpu, counter) (cpu->csr.scounteren & (1<<counter))
#define is_high_counter(csr) (csr & 0x080)

static inline void read_counter_csr_unchecked(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    int counter = csr & 0x1F;

    int offset = is_high_counter(csr) ? 32 : 0;

    switch(counter){
        case (csr_cycle & 0x1F): {
            *target = EXTRACT_BITS(cpu->csr.cycle, offset, offset + 32);
            break;
        }
        case (csr_time & 0x1F): {
            *target = EXTRACT_BITS(cpu->csr.mtime, offset, offset + 32);
            break;
        }
        case (csr_instret & 0x1F): {
            *target = EXTRACT_BITS(cpu->csr.instret, offset, offset + 32);
            break;
        }
        default: {
            uint64_t hpc = cpu->csr.hpmcounters[counter - 3];
            *target = EXTRACT_BITS(hpc, offset, offset + 32);
            break;
        }
    }
}

static rv_exc_t counter_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){

    // lowest 5 bits
    int counter = csr * 0x1F;

    if(rv_csr_min_priv_mode(csr) != rv_mmode){

        if(cpu->priv_mode == rv_smode && !is_counter_enabled_m(cpu, counter))
            return rv_exc_illegal_instruction;
        
        if(cpu->priv_mode == rv_umode && !(is_counter_enabled_m(cpu, counter) && is_counter_enabled_s(cpu, counter)))
            return rv_exc_illegal_instruction;

    }
    else if (cpu->priv_mode != rv_mmode){
        return rv_exc_illegal_instruction;
    }
    else if (counter == (csr_time & 0x1F)){
        // mtime is not a csr
        return rv_exc_illegal_instruction;
    }

    read_counter_csr_unchecked(cpu, csr, target);

    return rv_exc_none;
}

static rv_exc_t counter_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){

    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = 0;
    uint64_t mask = 0;

    if(is_high_counter(csr)){
        val = ((uint64_t)value) << 32;
        mask = 0x00000000FFFFFFFF;
    }
    else {
        val = value;
        mask = 0xFFFFFFFF00000000;
    }

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle = (cpu->csr.cycle & mask) | val;
            cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret = (cpu->csr.instret & mask) | val;
            break;
        }
        default: {
            uint64_t hpc = cpu->csr.hpmcounters[counter - 3];
            cpu->csr.hpmcounters[counter - 3] = (hpc & mask) | val;
            break;
        }
    }

    return rv_exc_none;
}

static rv_exc_t counter_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){

    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = is_high_counter(csr) ? ((uint64_t)value) << 32 : value;

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle |= val;
            cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret |= val;
            break;
        }
        default: {
            cpu->csr.hpmcounters[counter - 3] |= val;
            break;
        }
    }

    return rv_exc_none;
}

static rv_exc_t counter_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = is_high_counter(csr) ? ((uint64_t)value) << 32 : value;

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle &= ~val;
            cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret &= ~val;
            break;
        }
        default: {
            cpu->csr.hpmcounters[counter - 3] &= ~val;
            break;
        }
    }

    return rv_exc_none;
}

#define mcountinhibit_mask (~UINT32_C(0b10))

static rv_exc_t mcountinhibit_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mcountinhibit;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit = value & mcountinhibit_mask;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit |= value & mcountinhibit_mask;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit &= ~(value & mcountinhibit_mask);
    return rv_exc_none;
}

static rv_exc_t mhpmevent_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    int event = (csr & 0x1F) - 3;
    *target = cpu->csr.hpmevents[event];
    return rv_exc_none;
}

static rv_exc_t mhpmevent_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    int event = (csr & 0x1F) - 3;

    if (value < hpm_event_count){
        cpu->csr.hpmevents[event] = value;
    }
    return rv_exc_none;
}

static rv_exc_t mhpmevent_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    int event = (csr & 0x1F) - 3;

    int val = cpu->csr.hpmevents[event] | value;

    if (val < hpm_event_count){
        cpu->csr.hpmevents[event] = val;
    }
    return rv_exc_none;
}

static rv_exc_t mhpmevent_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    int event = (csr & 0x1F) - 3;

    int val = cpu->csr.hpmevents[event] & ~value;

    if (val < hpm_event_count){
        cpu->csr.hpmevents[event] = val;
    }

    return rv_exc_none;
}

static rv_exc_t pmpcfg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpcfg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpcfg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpcfg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpaddr_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpaddr_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpaddr_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t pmpaddr_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t sstatus_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.mstatus & rv_csr_sstatus_mask;
    return rv_exc_none;
}

static rv_exc_t sstatus_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    
    // Masked write of low 32 bits
    cpu->csr.mstatus = (cpu->csr.mstatus & 0xFFFFFFFF00000000) | (value & rv_csr_sstatus_mask);
    return rv_exc_none;
}

static rv_exc_t sstatus_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    // Masked write
    cpu->csr.mstatus |= (value & rv_csr_sstatus_mask);
    return rv_exc_none;
}

static rv_exc_t sstatus_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    // Masked write
    cpu->csr.mstatus &= ~(value & rv_csr_sstatus_mask);
    return rv_exc_none;
}

static rv_exc_t sie_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.mie & rv_csr_si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    
    // write only to si bits, preserve rest
    cpu->csr.mie &= ~rv_csr_si_mask;
    cpu->csr.mie |= value & rv_csr_si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mie |= value & rv_csr_si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mie &= ~(value & rv_csr_si_mask);
    return rv_exc_none;
}

static rv_exc_t sip_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    
    // The SEIP bit is the logical OR of the value in MIP and the status from external interrupt controller
    // Full explanation RISC-V Privileged spec section 3.1.9 Machine Interrupt Registers (mip and mie)
    *target = (cpu->csr.mip & rv_csr_si_mask)
              | (cpu->csr.external_SEIP ? rv_csr_sei_mask : 0)
              | (cpu->csr.external_STIP ? rv_csr_sti_mask : 0);
    return rv_exc_none;
}

static rv_exc_t sip_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    
    // only ssip is writable
    cpu->csr.mip &= ~rv_csr_ssi_mask;
    cpu->csr.mip |= value & rv_csr_ssi_mask;
    return rv_exc_none;
}

static rv_exc_t sip_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mip |= value & rv_csr_ssi_mask;
    return rv_exc_none;
}

static rv_exc_t sip_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mip &= ~(value & rv_csr_ssi_mask);
    return rv_exc_none;
}

#define scvec_mask 0xFFFFFFFD

static rv_exc_t stvec_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.stvec;
    return rv_exc_none;
}

static rv_exc_t stvec_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec = value & scvec_mask;
    return rv_exc_none;
}

static rv_exc_t stvec_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec |= value & scvec_mask;
    return rv_exc_none;
}

static rv_exc_t stvec_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec &= ~(value & scvec_mask);
    return rv_exc_none;
}

default_csr_functions(scounteren, rv_smode)

#define senvcfg_mask 0x71

static rv_exc_t senvcfg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.senvcfg;
    return rv_exc_none;
}

static rv_exc_t senvcfg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.senvcfg = value & senvcfg_mask;
    return rv_exc_none;
}

static rv_exc_t senvcfg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.senvcfg |= value & senvcfg_mask;
    return rv_exc_none;
}

static rv_exc_t senvcfg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.senvcfg &= ~(value & senvcfg_mask);
    return rv_exc_none;
}

default_csr_functions(sscratch, rv_smode)

#define sepc_mask 0xFFFFFFFC

static rv_exc_t sepc_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.sepc;
    return rv_exc_none;
}

static rv_exc_t sepc_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc = value & sepc_mask;
    return rv_exc_none;
}

static rv_exc_t sepc_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc |= value & sepc_mask;
    return rv_exc_none;
}

static rv_exc_t sepc_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc &= ~(value & sepc_mask);
    return rv_exc_none;
}

static bool is_exception_code(uint32_t num){
    if(RV_INTERRUPT_EXC_BITS & num){
        return RV_EXCEPTION_MASK(num) & RV_INTERRUPTS_MASK;
    }
    return RV_EXCEPTION_MASK(num) & RV_EXCEPTIONS_MASK;
}


static rv_exc_t scause_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.scause;
    return rv_exc_none;
}

static rv_exc_t scause_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);

    if(is_exception_code(value)){
        cpu->csr.scause = value;
    }
    else {
        return rv_exc_illegal_instruction;
    }

    return rv_exc_none;
}

static rv_exc_t scause_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);

    uint32_t val = value | cpu->csr.scause;
    if(is_exception_code(val)){
        cpu->csr.scause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }

    return rv_exc_none;
}

static rv_exc_t scause_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);

    uint32_t val = ~value & cpu->csr.scause;
    if(is_exception_code(val)){
        cpu->csr.scause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }
    
    return rv_exc_none;
}

default_csr_functions(stval, rv_smode)

static rv_exc_t satp_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    if(rv_csr_mstatus_tvm(cpu)) return rv_exc_illegal_instruction;

    *target = cpu->csr.satp;
    return rv_exc_none;
}

static rv_exc_t satp_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    if(rv_csr_mstatus_tvm(cpu)) return rv_exc_illegal_instruction;

    cpu->csr.satp = value;
    
    if(rv_csr_satp_is_bare(cpu)){
        cpu->csr.satp = 0;
    }
    
    return rv_exc_none;
}

static rv_exc_t satp_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    if(rv_csr_mstatus_tvm(cpu)) return rv_exc_illegal_instruction;

    cpu->csr.satp |= value;

    if(rv_csr_satp_is_bare(cpu)){
        cpu->csr.satp = 0;
    }

    return rv_exc_none;
}

static rv_exc_t satp_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    if(rv_csr_mstatus_tvm(cpu)) return rv_exc_illegal_instruction;

    cpu->csr.satp &= value;

    if(rv_csr_satp_is_bare(cpu)){
        cpu->csr.satp = 0;
    }

    return rv_exc_none;
}

default_csr_functions(scontext, rv_smode)

// Writes to scyclecmp request or unrequest STI (based on the low 32 bits of cycle)

static rv_exc_t scyclecmp_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.scyclecmp;
    return rv_exc_none;
}

static rv_exc_t scyclecmp_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.scyclecmp = value;
    cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
    return rv_exc_none;
}
static rv_exc_t scyclecmp_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.scyclecmp |= value;
    cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
    return rv_exc_none;
}
static rv_exc_t scyclecmp_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.scyclecmp &= ~value;
    cpu->csr.external_STIP = ((uint32_t)cpu->csr.cycle) >= cpu->csr.scyclecmp;
    return rv_exc_none;
}


static rv_exc_t mvendorid_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mvendorid;
    return rv_exc_none;
}

static rv_exc_t marchid_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.marchid;
    return rv_exc_none;
}

static rv_exc_t mimpid_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mimpid;
    return rv_exc_none;
}

static rv_exc_t mhartid_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mhartid;
    return rv_exc_none;
}

static rv_exc_t mconfigptr_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mconfigptr;
    return rv_exc_none;
}



static rv_exc_t mstatus_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = (uint32_t)cpu->csr.mstatus;
    return rv_exc_none;
}

static rv_exc_t mstatus_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);

    uint64_t val = value & rv_csr_mstatus_mask;

    // if the new mpp mode would be rmode (reserved, invalid value), don't modify it
    if(((val & rv_csr_mstatus_mpp_mask) >> 11) == rv_rmode){
        // zero out mpp bits from val
        val &= ~rv_csr_mstatus_mpp_mask;
        // set the bits to the current value
        val |= ((uint32_t)cpu->csr.mstatus) & rv_csr_mstatus_mpp_mask;
    }

    cpu->csr.mstatus = (cpu->csr.mstatus & 0xFFFFFFFF00000000) | val;
    return rv_exc_none;
}

static rv_exc_t mstatus_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    
    value &= rv_csr_mstatus_mask;

    uint64_t new_val = ((uint64_t)value) | cpu->csr.mstatus;

    // if the new mpp mode would be rmode (reserved, invalid value), don't modify it
    if(((new_val & rv_csr_mstatus_mpp_mask) >> 11) == rv_rmode){
        value &= ~rv_csr_mstatus_mpp_mask;
    }

    cpu->csr.mstatus |= value;

    return rv_exc_none;
}

static rv_exc_t mstatus_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);

    value &= rv_csr_mstatus_mask;
    // Masked write
    uint64_t new_val = (~(uint64_t)value) & cpu->csr.mstatus;

    // if the new mpp mode would be rmode (reserved, invalid value), don't modify it
    if(((new_val & rv_csr_mstatus_mpp_mask) >> 11) == rv_rmode){
        value &= ~rv_csr_mstatus_mpp_mask;
    }

    cpu->csr.mstatus &= ~value;
    return rv_exc_none;
}

// Since MBE and SBE are both R/O zero and other bits re WPRI, whole mstatush is R/O zero
static rv_exc_t mstatush_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = (uint32_t)(cpu->csr.mstatus >> 32);
    return rv_exc_none;
}

static rv_exc_t mstatush_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    // writes to mstatush have no effect
    return rv_exc_none;
}

static rv_exc_t mstatush_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    // writes to mstatush have no effect
    return rv_exc_none;
}

static rv_exc_t mstatush_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    // writes to mstatush have no effect
    return rv_exc_none;
}

static rv_exc_t misa_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.misa;
    return rv_exc_none;
}

// misa writes do nothing, we don't allow the change of extensions or MXLEN
static rv_exc_t misa_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t misa_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t misa_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}


// mmode ecall can't be delegated
#define medeleg_mask (RV_EXCEPTIONS_MASK & ~RV_EXCEPTION_MASK(rv_exc_mmode_environment_call))

static rv_exc_t medeleg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.medeleg;
    return rv_exc_none;
}

static rv_exc_t medeleg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.medeleg = value & medeleg_mask;
    return rv_exc_none;
}

static rv_exc_t medeleg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.medeleg |= value & medeleg_mask;
    return rv_exc_none;
}

static rv_exc_t medeleg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.medeleg &= ~(value & medeleg_mask);
    return rv_exc_none;
}

static rv_exc_t mideleg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mideleg;
    return rv_exc_none;
}


// we allow only smode interrupts to be delegatable
static rv_exc_t mideleg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mideleg = value & rv_csr_si_mask;
    return rv_exc_none;
}

static rv_exc_t mideleg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mideleg |= value & rv_csr_si_mask;
    return rv_exc_none;
}

static rv_exc_t mideleg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mideleg &= ~(value & rv_csr_si_mask);
    return rv_exc_none;
}

static rv_exc_t mie_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mie;
    return rv_exc_none;
}

static rv_exc_t mie_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mie = value & rv_csr_mi_mask;
    return rv_exc_none;
}

static rv_exc_t mie_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mie |= value & rv_csr_mi_mask;
    return rv_exc_none;
}

static rv_exc_t mie_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mie &= ~(value & rv_csr_mi_mask);
    return rv_exc_none;
}

static rv_exc_t mip_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);

    // The SEIP bit is the logical OR of the value in MIP and the status from external interrupt controller
    // Full explanation RISC-V Privileged spec section 3.1.9 Machine Interrupt Registers (mip and mie)
    *target = cpu->csr.mip
              | (cpu->csr.external_SEIP ? rv_csr_sei_mask : 0)
              | (cpu->csr.external_STIP ? rv_csr_sti_mask : 0)
              ;
    return rv_exc_none;
}

// M-mode interrupts are not directly writable, but all S-mode interrupts are
#define mip_mask rv_csr_si_mask

static rv_exc_t mip_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);

    cpu->csr.mip = value & mip_mask;
    return rv_exc_none;
}

static rv_exc_t mip_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mip |= value & mip_mask;
    return rv_exc_none;
}

static rv_exc_t mip_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mip &= ~(value & mip_mask);
    return rv_exc_none;
}

#define mtvec_mask (~UINT32_C(2))

static rv_exc_t mtvec_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mtvec;
    return rv_exc_none;
}

static rv_exc_t mtvec_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mtvec = value & mtvec_mask;
    return rv_exc_none;
}

static rv_exc_t mtvec_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mtvec |= value & mtvec_mask;
    return rv_exc_none;
}

static rv_exc_t mtvec_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mtvec &= ~(value & mtvec_mask);
    return rv_exc_none;
}

default_csr_functions(mcounteren, rv_mmode)


default_csr_functions(mscratch, rv_mmode)
default_csr_functions(mepc, rv_mmode)

static rv_exc_t mcause_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mcause;
    return rv_exc_none;
}

static rv_exc_t mcause_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    if(is_exception_code(value)){
        cpu->csr.mcause = value;
    }
    else {
        return rv_exc_illegal_instruction;
    }
    return rv_exc_none;
}

static rv_exc_t mcause_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);

    uint32_t val = cpu->csr.mcause | value;

    if(is_exception_code(val)){
        cpu->csr.mcause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }
    return rv_exc_none;
}

static rv_exc_t mcause_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    uint32_t val = cpu->csr.mcause & ~value;

    if(is_exception_code(val)){
        cpu->csr.mcause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }
    return rv_exc_none;
}

default_csr_functions(mtval, rv_mmode)

static rv_exc_t mtinst_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtinst_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtinst_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtinst_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtval2_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtval2_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtval2_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mtval2_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

#define menvcfg_fiom_mask UINT32_C(1)

static rv_exc_t menvcfg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = (uint32_t)cpu->csr.menvcfg;
    return rv_exc_none;
}

static rv_exc_t menvcfg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.menvcfg = (cpu->csr.menvcfg & 0xFFFFFFFF00000000) | (value & menvcfg_fiom_mask);
    return rv_exc_none;
}

static rv_exc_t menvcfg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.menvcfg |= (uint64_t)(value & menvcfg_fiom_mask);
    return rv_exc_none;
}

static rv_exc_t menvcfg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    // Writing to unwritable fields
    cpu->csr.menvcfg &= ~((uint64_t)(value & menvcfg_fiom_mask));
    return rv_exc_none;
}

static rv_exc_t menvcfgh_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.menvcfg >> 32;
    return rv_exc_none;
}

static rv_exc_t menvcfgh_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t menvcfgh_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t menvcfgh_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

// mseccfg(h) do nothing as of now, so they are read-only 0
static rv_exc_t mseccfg_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mseccfg;
    return rv_exc_none;
}

static rv_exc_t mseccfg_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t mseccfg_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t mseccfg_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t mseccfgh_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = 0;
    return rv_exc_none;
}

static rv_exc_t mseccfgh_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t mseccfgh_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t mseccfgh_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    minimal_privilege(rv_mmode, cpu);
    return rv_exc_none;
}

static rv_exc_t tselect_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tselect_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tselect_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tselect_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata1_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata1_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata1_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata1_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata2_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata2_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata2_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata2_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata3_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata3_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata3_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t tdata3_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mcontext_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mcontext_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mcontext_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t mcontext_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dcsr_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dcsr_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dcsr_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dcsr_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dpc_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dpc_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dpc_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dpc_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch0_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch0_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch0_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch0_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch1_read(rv_cpu_t* cpu, csr_num_t csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch1_write(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch1_set(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

static rv_exc_t dscratch1_clear(rv_cpu_t* cpu, csr_num_t csr, uint32_t value){
    return rv_exc_illegal_instruction;
}

typedef rv_exc_t (*csr_read_func_t)(rv_cpu_t*, csr_num_t csr, uint32_t*);
typedef rv_exc_t (*csr_write_func_t)(rv_cpu_t*, csr_num_t csr, uint32_t);
typedef rv_exc_t (*csr_set_func_t)(rv_cpu_t*, csr_num_t csr, uint32_t);
typedef rv_exc_t (*csr_clear_func_t)(rv_cpu_t*, csr_num_t csr, uint32_t);

/**
 * Structure holding the basic CSR operations
 */
typedef struct {
    csr_read_func_t read; /** Reads the whole CSR */
    csr_write_func_t write; /** Writes the whole CSR */
    csr_set_func_t set; /** Sets the bits on position that has a bit set in the given value (writing only to those bits)*/
    csr_clear_func_t clear; /** Clears the bits on position that has a bit set in the given value (writing only to those bits)*/
} csr_ops_t;

/** Retrieves the CSR ops for the given csr*/
static csr_ops_t get_csr_ops(csr_num_t csr){
    csr_ops_t ops = {
        .read = invalid_read,
        .write = invalid_write,
        .set = invalid_write,
        .clear = invalid_write
    };

    #define default_case(csr)           \
    case csr_##csr: {                   \
        ops.read = csr##_read;          \
        ops.write = csr##_write;        \
        ops.set = csr##_set;            \
        ops.clear = csr##_clear;        \
        break;                          \
    }                                   \

    #define read_only_case(csr)         \
    case csr_##csr: {                   \
        ops.read = csr##_read;          \
        ops.write = invalid_write;      \
        ops.set = invalid_write;        \
        ops.clear = invalid_write;      \
        break;                          \
    }                            

    switch(csr){
        case(csr_cycle):
        case(csr_time):
        case(csr_instret):
        case(csr_hpmcounter3):
        case(csr_hpmcounter4):
        case(csr_hpmcounter5):
        case(csr_hpmcounter6):
        case(csr_hpmcounter7):
        case(csr_hpmcounter8):
        case(csr_hpmcounter9):
        case(csr_hpmcounter10):
        case(csr_hpmcounter11):
        case(csr_hpmcounter12):
        case(csr_hpmcounter13):
        case(csr_hpmcounter14):
        case(csr_hpmcounter15):
        case(csr_hpmcounter16):
        case(csr_hpmcounter17):
        case(csr_hpmcounter18):
        case(csr_hpmcounter19):
        case(csr_hpmcounter20):
        case(csr_hpmcounter21):
        case(csr_hpmcounter22):
        case(csr_hpmcounter23):
        case(csr_hpmcounter24):
        case(csr_hpmcounter25):
        case(csr_hpmcounter26):
        case(csr_hpmcounter27):
        case(csr_hpmcounter28):
        case(csr_hpmcounter29):
        case(csr_hpmcounter30):
        case(csr_hpmcounter31):
        case(csr_cycleh):
        case(csr_timeh):
        case(csr_instreth):
        case(csr_hpmcounter3h):
        case(csr_hpmcounter4h):
        case(csr_hpmcounter5h):
        case(csr_hpmcounter6h):
        case(csr_hpmcounter7h):
        case(csr_hpmcounter8h):
        case(csr_hpmcounter9h):
        case(csr_hpmcounter10h):
        case(csr_hpmcounter11h):
        case(csr_hpmcounter12h):
        case(csr_hpmcounter13h):
        case(csr_hpmcounter14h):
        case(csr_hpmcounter15h):
        case(csr_hpmcounter16h):
        case(csr_hpmcounter17h):
        case(csr_hpmcounter18h):
        case(csr_hpmcounter19h):
        case(csr_hpmcounter20h):
        case(csr_hpmcounter21h):
        case(csr_hpmcounter22h):
        case(csr_hpmcounter23h):
        case(csr_hpmcounter24h):
        case(csr_hpmcounter25h):
        case(csr_hpmcounter26h):
        case(csr_hpmcounter27h):
        case(csr_hpmcounter28h):
        case(csr_hpmcounter29h):
        case(csr_hpmcounter30h):
        case(csr_hpmcounter31h):
        case(csr_mcycle):
        case(csr_minstret):
        case(csr_mhpmcounter3):
        case(csr_mhpmcounter4):
        case(csr_mhpmcounter5):
        case(csr_mhpmcounter6):
        case(csr_mhpmcounter7):
        case(csr_mhpmcounter8):
        case(csr_mhpmcounter9):
        case(csr_mhpmcounter10):
        case(csr_mhpmcounter11):
        case(csr_mhpmcounter12):
        case(csr_mhpmcounter13):
        case(csr_mhpmcounter14):
        case(csr_mhpmcounter15):
        case(csr_mhpmcounter16):
        case(csr_mhpmcounter17):
        case(csr_mhpmcounter18):
        case(csr_mhpmcounter19):
        case(csr_mhpmcounter20):
        case(csr_mhpmcounter21):
        case(csr_mhpmcounter22):
        case(csr_mhpmcounter23):
        case(csr_mhpmcounter24):
        case(csr_mhpmcounter25):
        case(csr_mhpmcounter26):
        case(csr_mhpmcounter27):
        case(csr_mhpmcounter28):
        case(csr_mhpmcounter29):
        case(csr_mhpmcounter30):
        case(csr_mhpmcounter31):
        case(csr_mcycleh):
        case(csr_minstreth):
        case(csr_mhpmcounter3h):
        case(csr_mhpmcounter4h):
        case(csr_mhpmcounter5h):
        case(csr_mhpmcounter6h):
        case(csr_mhpmcounter7h):
        case(csr_mhpmcounter8h):
        case(csr_mhpmcounter9h):
        case(csr_mhpmcounter10h):
        case(csr_mhpmcounter11h):
        case(csr_mhpmcounter12h):
        case(csr_mhpmcounter13h):
        case(csr_mhpmcounter14h):
        case(csr_mhpmcounter15h):
        case(csr_mhpmcounter16h):
        case(csr_mhpmcounter17h):
        case(csr_mhpmcounter18h):
        case(csr_mhpmcounter19h):
        case(csr_mhpmcounter20h):
        case(csr_mhpmcounter21h):
        case(csr_mhpmcounter22h):
        case(csr_mhpmcounter23h):
        case(csr_mhpmcounter24h):
        case(csr_mhpmcounter25h):
        case(csr_mhpmcounter26h):
        case(csr_mhpmcounter27h):
        case(csr_mhpmcounter28h):
        case(csr_mhpmcounter29h):
        case(csr_mhpmcounter30h):
        case(csr_mhpmcounter31h): {
            ops.read = counter_read;
            ops.write = counter_write;
            ops.set = counter_set;
            ops.clear = counter_clear;
            break;
        }

        case(csr_mhpmevent3):
        case(csr_mhpmevent4):
        case(csr_mhpmevent5):
        case(csr_mhpmevent6):
        case(csr_mhpmevent7):
        case(csr_mhpmevent8):
        case(csr_mhpmevent9):
        case(csr_mhpmevent10):
        case(csr_mhpmevent11):
        case(csr_mhpmevent12):
        case(csr_mhpmevent13):
        case(csr_mhpmevent14):
        case(csr_mhpmevent15):
        case(csr_mhpmevent16):
        case(csr_mhpmevent17):
        case(csr_mhpmevent18):
        case(csr_mhpmevent19):
        case(csr_mhpmevent20):
        case(csr_mhpmevent21):
        case(csr_mhpmevent22):
        case(csr_mhpmevent23):
        case(csr_mhpmevent24):
        case(csr_mhpmevent25):
        case(csr_mhpmevent26):
        case(csr_mhpmevent27):
        case(csr_mhpmevent28):
        case(csr_mhpmevent29):
        case(csr_mhpmevent30):
        case(csr_mhpmevent31): {
            ops.read = mhpmevent_read;
            ops.write = mhpmevent_write;
            ops.set = mhpmevent_set;
            ops.clear = mhpmevent_clear;
            break;
        }

        case(csr_pmpcfg0):
	    case(csr_pmpcfg1):
	    case(csr_pmpcfg2):
	    case(csr_pmpcfg3):
	    case(csr_pmpcfg4):
	    case(csr_pmpcfg5):
	    case(csr_pmpcfg6):
	    case(csr_pmpcfg7):
	    case(csr_pmpcfg8):
	    case(csr_pmpcfg9):
	    case(csr_pmpcfg10):
	    case(csr_pmpcfg11):
	    case(csr_pmpcfg12):
	    case(csr_pmpcfg13):
	    case(csr_pmpcfg14):
	    case(csr_pmpcfg15): {
            ops.read = pmpcfg_read;
            ops.write = pmpcfg_write;
            ops.set = pmpcfg_set;
            ops.clear = pmpcfg_clear;
            break;
        }

        case(csr_pmpaddr0):
        case(csr_pmpaddr1):
        case(csr_pmpaddr2):
        case(csr_pmpaddr3):
        case(csr_pmpaddr4):
        case(csr_pmpaddr5):
        case(csr_pmpaddr6):
        case(csr_pmpaddr7):
        case(csr_pmpaddr8):
        case(csr_pmpaddr9):
        case(csr_pmpaddr10):
        case(csr_pmpaddr11):
        case(csr_pmpaddr12):
        case(csr_pmpaddr13):
        case(csr_pmpaddr14):
        case(csr_pmpaddr15):
        case(csr_pmpaddr16):
        case(csr_pmpaddr17):
        case(csr_pmpaddr18):
        case(csr_pmpaddr19):
        case(csr_pmpaddr20):
        case(csr_pmpaddr21):
        case(csr_pmpaddr22):
        case(csr_pmpaddr23):
        case(csr_pmpaddr24):
        case(csr_pmpaddr25):
        case(csr_pmpaddr26):
        case(csr_pmpaddr27):
        case(csr_pmpaddr28):
        case(csr_pmpaddr29):
        case(csr_pmpaddr30):
        case(csr_pmpaddr31):
        case(csr_pmpaddr32):
        case(csr_pmpaddr33):
        case(csr_pmpaddr34):
        case(csr_pmpaddr35):
        case(csr_pmpaddr36):
        case(csr_pmpaddr37):
        case(csr_pmpaddr38):
        case(csr_pmpaddr39):
        case(csr_pmpaddr40):
        case(csr_pmpaddr41):
        case(csr_pmpaddr42):
        case(csr_pmpaddr43):
        case(csr_pmpaddr44):
        case(csr_pmpaddr45):
        case(csr_pmpaddr46):
        case(csr_pmpaddr47):
        case(csr_pmpaddr48):
        case(csr_pmpaddr49):
        case(csr_pmpaddr50):
        case(csr_pmpaddr51):
        case(csr_pmpaddr52):
        case(csr_pmpaddr53):
        case(csr_pmpaddr54):
        case(csr_pmpaddr55):
        case(csr_pmpaddr56):
        case(csr_pmpaddr57):
        case(csr_pmpaddr58):
        case(csr_pmpaddr59):
        case(csr_pmpaddr60):
        case(csr_pmpaddr61):
        case(csr_pmpaddr62):
        case(csr_pmpaddr63):{
            ops.read = pmpaddr_read;
            ops.write = pmpaddr_write;
            ops.set = pmpaddr_set;
            ops.clear = pmpaddr_clear;
            break;
        }

        default_case(mcountinhibit)

        default_case(sstatus)
    
        default_case(sie)
        default_case(stvec)
        default_case(scounteren)
    
        default_case(senvcfg)
    
        default_case(sscratch)
        default_case(sepc)
        default_case(scause)
        default_case(stval)
        default_case(sip)
        default_case(satp)
        default_case(scontext)
        default_case(scyclecmp)
        
        read_only_case(mvendorid)
        read_only_case(marchid)
        read_only_case(mimpid)
        read_only_case(mhartid)
        read_only_case(mconfigptr)

        default_case(mstatus)
    
        default_case(misa)
        default_case(medeleg)
        default_case(mideleg)
        default_case(mie)
        default_case(mtvec)
        default_case(mcounteren)
    
        default_case(mstatush)
    
        default_case(mscratch)
        default_case(mepc)
        default_case(mcause)
        default_case(mtval)
        default_case(mip)
        default_case(mtinst)
        default_case(mtval2)
    
        default_case(menvcfg)
        default_case(menvcfgh)
        default_case(mseccfg)
        default_case(mseccfgh)
    
        default_case(tselect)
        default_case(tdata1)
        default_case(tdata2)
        default_case(tdata3)
        default_case(mcontext)
        default_case(dcsr)
        default_case(dpc)
        default_case(dscratch0)
        default_case(dscratch1)
    }

    return ops;

    #undef default_case
    #undef read_only_case
}

/**
 * @brief Reads the old value from the CSR, then writes the specified value
 * 
 * @param cpu The cpu on which this operation is done
 * @param csr The csr on which this operation is done
 * @param value The value to be written
 * @param read_target The location where the read original value will be stored on success
 * @param read Whether the read should be done
 * @return rv_exc_t The exception code
 */
rv_exc_t rv_csr_rw(rv_cpu_t* cpu, csr_num_t csr, uint32_t value, uint32_t* read_target, bool read){

    csr_ops_t ops = get_csr_ops(csr);
    rv_exc_t ex = rv_exc_none;
    uint32_t temp_read_target = 0;

    if(read){
        ex = ops.read(cpu, csr, &temp_read_target);
    }

    if(ex == rv_exc_none){
        ex = ops.write(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}

/**
 * @brief Reads the old value from the CSR, then sets the specified bits
 * 
 * @param cpu The cpu on which this operation is done
 * @param csr The csr on which this operation is done
 * @param value The value specifying which bits to set
 * @param read_target The location where the read original value will be stored on success
 * @param write Whether the bits should be set
 * @return rv_exc_t The exception code
 */
rv_exc_t rv_csr_rs(rv_cpu_t* cpu, csr_num_t csr, uint32_t value, uint32_t* read_target, bool write){
    csr_ops_t ops = get_csr_ops(csr);
    
    uint32_t temp_read_target = 0;

    rv_exc_t ex = ops.read(cpu, csr, &temp_read_target);

    if(ex == rv_exc_none && write){
        ex = ops.set(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}

/**
 * @brief Reads the old value from the CSR, then clears the specified bits
 * 
 * @param cpu The cpu on which this operation is done
 * @param csr The csr on which this operation is done
 * @param value The value specifying which bits to clear
 * @param read_target The location where the read original value will be stored on success
 * @param write Whether the bits should be cleared
 * @return rv_exc_t The exception code
 */
rv_exc_t rv_csr_rc(rv_cpu_t* cpu, csr_num_t csr, uint32_t value, uint32_t* read_target, bool write){
   
    csr_ops_t ops = get_csr_ops(csr);
    uint32_t temp_read_target = 0;
    rv_exc_t ex = ops.read(cpu, csr, &temp_read_target);
    
    if(ex == rv_exc_none && write){
        ex = ops.clear(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}

/**
 * @brief The minimal privilege from which the given csr can be accessed
 */
rv_priv_mode_t rv_csr_min_priv_mode(csr_num_t csr) {
    return (rv_priv_mode_t)(((unsigned int)csr >> 8) & 0b11);
}