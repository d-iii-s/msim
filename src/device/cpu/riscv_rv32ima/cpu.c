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
    cpu->pc_next = RV_START_ADDRESS + 4;
}


void rv_cpu_init(rv_cpu_t *cpu, unsigned int procno){

    ASSERT(cpu!=NULL);

    memset(cpu, 0, sizeof(rv_cpu_t));

    init_regs(cpu);

    init_csr(&cpu->csr, procno);
   
    printf("Initialized rv cpu id %u\n", cpu->csr.mhartid);
}   


rv_exc_t rv_convert_addr(rv_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool noisy){
    ASSERT(cpu != NULL);
    ASSERT(phys != NULL);

    *phys = virt;

    return rv_exc_none;
}

rv_exc_t rv_read_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t *value, bool noisy){
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);
    //TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    *value = physmem_read32(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

rv_exc_t rv_write_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t value, bool noisy){
    ASSERT(cpu != NULL);
    // TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    if(physmem_write32(cpu->csr.mhartid, phys, value, true)){
        return rv_exc_none;
    }
    //TODO: handle write into invalid memory
    return rv_exc_none;
}






void rv_cpu_set_pc(rv_cpu_t *cpu, uint32_t value){

}















void rv_cpu_step(rv_cpu_t *cpu){

    ptr36_t phys;
    rv_exc_t ex;
    while((ex = rv_convert_addr(cpu, cpu->pc, &phys, false, true)) != rv_exc_none){
        // TODO: handle exception
    }

    rv_instr_t instr_data = (rv_instr_t)physmem_read32(cpu->csr.mhartid, phys, false);

    printf("Cycle: %ld\tpc: 0x%08x\tmem: 0x%08x ", cpu->csr.cycle, cpu->pc, instr_data.val);
    
    rv_instr_func_t instr_func = rv_instr_decode(instr_data);
    instr_func(cpu, instr_data);

    printf("\n");

    cpu->csr.cycle++;

    // x0 is always 0
    cpu->regs[0] = 0;
    cpu->pc = cpu->pc_next;
    cpu->pc_next = cpu->pc + 4;
    
}

/** Interrupts */
void rv_cpu_interrupt_up(rv_cpu_t *cpu, unsigned int no){

}

void rv_cpu_interrupt_down(rv_cpu_t *cpu, unsigned int no){

}