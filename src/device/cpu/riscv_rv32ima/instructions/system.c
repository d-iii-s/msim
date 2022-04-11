#include <stdint.h>
#include "system.h"
#include "../../../../assert.h"

rv_exc_t break_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);

    printf("BREAK");

    alert("BREAK");

    machine_interactive = true;
    return rv_exc_none;
}
rv_exc_t halt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    alert("HALT");

    machine_halt = true;
    return rv_exc_none;
}
