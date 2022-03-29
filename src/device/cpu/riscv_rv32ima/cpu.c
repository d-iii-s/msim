#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cpu.h"
#include "../../../assert.h"
#include "../../../physmem.h"
#include "../../../main.h"

#define RV_START_ADDRESS UINT32_C(0x0)


static void init_regs(rv_cpu_t *cpu) {
    // expects that default value for any variable is 0

    cpu->pc = RV_START_ADDRESS;
}


void rv_cpu_init(rv_cpu_t *cpu, unsigned int procno){

    ASSERT(cpu!=NULL);

    memset(cpu, 0, sizeof(rv_cpu_t));

    init_regs(cpu);

    init_csr(&cpu->csr, procno);
   
    printf("Initialized rv cpu id %u\n", cpu->csr.mhartid);
}   


static rv_exc_t rv_convert_addr(rv_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool noisy){
    ASSERT(cpu != NULL);
    ASSERT(phys != NULL);

    *phys = virt;

    return rv_exc_none;
}








void rv_cpu_set_pc(rv_cpu_t *cpu, uint32_t value){

}















void rv_cpu_step(rv_cpu_t *cpu){

    uint32_t val = physmem_read32(cpu->csr.mhartid, cpu->pc, false);

    printf("Cycle: %ld\tpc: 0x%08x\tmem: 0x%08x\n", cpu->csr.cycle, cpu->pc, val);
    
    cpu->csr.cycle++;
    cpu->pc += 4;

    if(val == UINT32_C(-1)){
        die(120, "reached end of memory!");
    }
}

/** Interrupts */
void rv_cpu_interrupt_up(rv_cpu_t *cpu, unsigned int no){

}

void rv_cpu_interrupt_down(rv_cpu_t *cpu, unsigned int no){

}