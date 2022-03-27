#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"


void rv32ima_cpu_init(rv32ima_cpu_t *cpu, unsigned int procno){
    cpu->csr.mhartid = procno;
    printf("Initialized rv cpu id %u\n", procno);
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