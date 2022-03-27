#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"


void rv32ima_cpu_init(rv32ima_cpu_t *cpu, unsigned int procno){
    memset(cpu, 0, sizeof(rv32ima_cpu_t));

    // init regs

    // init csr
    cpu->csr.mhartid = procno;

    // init other sstuff

    
    printf("Initialized rv cpu id %u\n", cpu->csr.mhartid);
}   

void rv32ima_cpu_set_pc(rv32ima_cpu_t *cpu, uint32_t value){

}

void rv32ima_cpu_step(rv32ima_cpu_t *cpu){
    printf("Stepping rv! Now on cycle %lu\n", cpu->csr.cycle);
    cpu->csr.cycle++;
}

/** Interrupts */
void rv32ima_cpu_interrupt_up(rv32ima_cpu_t *cpu, unsigned int no){

}

void rv32ima_cpu_interrupt_down(rv32ima_cpu_t *cpu, unsigned int no){

}