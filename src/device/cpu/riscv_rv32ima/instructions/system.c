#include <stdint.h>
#include "system.h"
#include "../../../../assert.h"

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
    // TODO: should return exception based on the current mode
    return rv_exc_none;
}

rv_exc_t sret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t mret_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t wfi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // Waiting for interrupt is simulated by stalling at this instruction
    // Even simpler solution would be to do a NOP (which is allowed by the standard)
    cpu->pc_next = cpu->pc;
    return rv_exc_none;
}

// Note: reads with rd = x0 shall not read the CSR and shall not have any side-efects based on the read

rv_exc_t csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

rv_exc_t sfence_instr(rv_cpu_t *cpu, rv_instr_t instr) {
    // This can be implemented as NOP in msim
    // TODO: Trap virtual memory
    return rv_exc_none;
    
}