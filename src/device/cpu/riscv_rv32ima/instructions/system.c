#include <stdint.h>
#include "system.h"
#include "../../../../assert.h"
#include "../csr.h"

rv_exc_t break_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    alert("EBREAK: breakpoint reached, entering interactive mode");

    machine_interactive = true;
    
    //TODO: this should return rv_exc_breakpoint by standard
    return rv_exc_none;
}
rv_exc_t halt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    alert("EHALT: Machine halt");

    machine_halt = true;
    return rv_exc_none;
}

rv_exc_t call_instr(rv_cpu_t *cpu, rv_instr_t instr){
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

rv_exc_t sret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    if(rv_csr_mstatus_tsr(cpu)){
        return rv_exc_illegal_instruction;
    }
    return rv_exc_none;
}

rv_exc_t mret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t wfi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    
    if(rv_csr_mstatus_tw(cpu) && cpu->priv_mode != rv_mmode){
        return rv_exc_illegal_instruction;
    }
    // Waiting for interrupt is simulated by stalling at this instruction
    // Even simpler solution would be to do a NOP (which is allowed by the standard)
    
    cpu->pc_next = cpu->pc;
    return rv_exc_none;
}

// Note: reads with rd = x0 shall not read the CSR and shall not have any side-efects based on the read

rv_exc_t csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd];  
    bool read = instr.i.rd != 0;

    return rv_csr_rw(cpu, csr, val, rd, read);
}

rv_exc_t csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd];  
    bool read = instr.i.rd != 0;

    return rv_csr_rs(cpu, csr, val, rd, read);
}

rv_exc_t csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = cpu->regs[instr.i.rs1];
    uint32_t* rd = &cpu->regs[instr.i.rd]; 
    bool read = instr.i.rd != 0;
 
    return rv_csr_rc(cpu, csr, val, rd, read);
}

rv_exc_t csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1;
    uint32_t* rd = &cpu->regs[instr.i.rd];  
    bool read = instr.i.rd != 0;

    return rv_csr_rw(cpu, csr, val, rd, read);
}

rv_exc_t csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1;
    uint32_t* rd = &cpu->regs[instr.i.rd];  
    bool read = instr.i.rd != 0;
    return rv_csr_rs(cpu, csr, val, rd, read);
}

rv_exc_t csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr){
    int csr = ((uint32_t)instr.i.imm) & 0xFFF;
    uint32_t val = instr.i.rs1;
    uint32_t* rd = &cpu->regs[instr.i.rd];  
    bool read = instr.i.rd != 0;

    return rv_csr_rc(cpu, csr, val, rd, read);
}

rv_exc_t sfence_instr(rv_cpu_t *cpu, rv_instr_t instr) {
    if(rv_csr_mstatus_tvm(cpu)){
        return rv_exc_illegal_instruction;
    }
    return rv_exc_none;
    
}