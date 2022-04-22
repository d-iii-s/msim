#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "../../../assert.h"
#include "../../../physmem.h"

#define RV_START_ADDRESS UINT32_C(0x0)

static void init_regs(rv32ima_cpu_t *cpu) {
    // expects that default value for any variable is 0

    cpu->pc = RV_START_ADDRESS;
}


void rv32ima_cpu_init(rv32ima_cpu_t *cpu, unsigned int procno){

    ASSERT(cpu!=NULL);

    memset(cpu, 0, sizeof(rv32ima_cpu_t));

    init_regs(cpu);

    init_csr(&cpu->csr, procno);
   
    printf("Initialized rv cpu id %u\n", cpu->csr.mhartid);
}   

void rv32ima_cpu_set_pc(rv32ima_cpu_t *cpu, uint32_t value){

}

void rv32ima_cpu_step(rv32ima_cpu_t *cpu){

    uint32_t val = physmem_read32(cpu->csr.mhartid, cpu->pc, false);

    printf("Cycle: %ld\tpc: 0x%08x\tmem: 0x%08x\n", cpu->csr.cycle, cpu->pc, val);
    
    cpu->csr.cycle++;
    cpu->pc += 4;

    if(val == UINT32_C(-1)){
        die(120, "reached end of memory!");
    }
}

/** Interrupts */
void rv32ima_cpu_interrupt_up(rv32ima_cpu_t *cpu, unsigned int no){

}

void rv32ima_cpu_interrupt_down(rv32ima_cpu_t *cpu, unsigned int no){

}