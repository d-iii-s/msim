#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cpu.h"
#include "debug.h"
#include "csr.h"
#include "../../../assert.h"
#include "../../../physmem.h"
#include "../../../main.h"

#define RV_START_ADDRESS UINT32_C(0x0)


static void init_regs(rv_cpu_t *cpu) {
    ASSERT(cpu != NULL);

    // expects that default value for any variable is 0

    cpu->pc = RV_START_ADDRESS;
    cpu->pc_next = RV_START_ADDRESS + 4;
}


void rv_cpu_init(rv_cpu_t *cpu, unsigned int procno){

    ASSERT(cpu!=NULL);

    memset(cpu, 0, sizeof(rv_cpu_t));

    init_regs(cpu);

    rv_init_csr(&cpu->csr, procno);

    cpu->priv_mode = rv_mmode;
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

rv_exc_t rv_read_mem16(rv_cpu_t *cpu, uint32_t virt, uint16_t *value, bool noisy){
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);
    //TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    *value = physmem_read16(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

rv_exc_t rv_read_mem8(rv_cpu_t *cpu, uint32_t virt, uint8_t *value, bool noisy){
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);
    //TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    *value = physmem_read8(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

rv_exc_t rv_write_mem8(rv_cpu_t *cpu, uint32_t virt, uint8_t value, bool noisy){
    ASSERT(cpu != NULL);
    // TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    if(physmem_write8(cpu->csr.mhartid, phys, value, true)){
        return rv_exc_none;
    }
    //TODO: handle write into invalid memory
    return rv_exc_none;

}

rv_exc_t rv_write_mem16(rv_cpu_t *cpu, uint32_t virt, uint16_t value, bool noisy){
    ASSERT(cpu != NULL);
    // TODO: check alignment

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, noisy);
    
    if(ex != rv_exc_none){
        return ex;
    }

    if(physmem_write16(cpu->csr.mhartid, phys, value, true)){
        return rv_exc_none;
    }
    //TODO: handle write into invalid memory
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
    ASSERT(cpu != NULL);
    /* Set both pc and pc_next
     * This should be called from the debugger to jump somewhere
     * and in case the new instruction does not modify pc_next,
     * the processor would then jump back to where it was before this call
     */
    cpu->pc = value;
    cpu->pc_next = value+4;    
}

static void handle_exception(rv_cpu_t* cpu, rv_exc_t ex){

}

static void try_handle_interrupt(rv_cpu_t* cpu){
    // no interrupt pending
    if(cpu->csr.mip == 0) return;

    // PRIORITY: MEI, MSI, MTI, SEI, SSI, STI



}



static void account_hmp(rv_cpu_t* cpu, int i){
    ASSERT((i >= 0 && i < 29));
    
    uint32_t mask = (1 << i);
    bool inhibited = cpu->csr.mcountinhibit & mask;
    
    if(inhibited) return;

    csr_hpm_event_t event = cpu->csr.hpmevents[i];

    switch(event){
        case(hpm_u_cycles):{
            if(cpu->priv_mode == rv_umode){
                cpu->csr.hpmcounters[i]++;
            }
            break;
        }
        case(hpm_s_cycles):{
            if(cpu->priv_mode == rv_smode){
                cpu->csr.hpmcounters[i]++;
            }
            break;
        }
        case(hpm_m_cycles):{
            if(cpu->priv_mode == rv_mmode){
                cpu->csr.hpmcounters[i]++;
            }
            break;
        }
        case(hpm_w_cycles):{
            // TODO: count stalled cycles (probably when the current instruction is WFI or cpu->stdby is true)
            break;
        }
        default:
            break;
    }
}

static void account(rv_cpu_t* cpu){
    if(!(cpu->csr.mcountinhibit & 0b001))
        cpu->csr.cycle++;
        
    if(!(cpu->csr.mcountinhibit & 0b100))
        cpu->csr.instret++;

    for(int i = 0; i < 29; ++i){
        account_hmp(cpu, i);
    }
}

void rv_cpu_step(rv_cpu_t *cpu){
    ASSERT(cpu != NULL);

    // If any interrupts are pending, handle them
    try_handle_interrupt(cpu);

    ptr36_t phys;
    rv_exc_t ex;
    while((ex = rv_convert_addr(cpu, cpu->pc, &phys, false, true)) != rv_exc_none){
        // TODO: handle exception
    }

    rv_instr_t instr_data = (rv_instr_t)physmem_read32(cpu->csr.mhartid, phys, false);

    rv_instr_func_t instr_func = rv_instr_decode(instr_data);
    
    if(machine_trace)
        rv_idump(cpu, cpu->pc, instr_data);
    
    ex = instr_func(cpu, instr_data);

    account(cpu);

    if(ex != rv_exc_none){
        handle_exception(cpu, ex);
    }

    // x0 is always 0
    cpu->regs[0] = 0;
    cpu->pc = cpu->pc_next;
    cpu->pc_next = cpu->pc + 4;
    
}

bool rv_sc_access(rv_cpu_t *cpu, ptr36_t phys){
    ASSERT(cpu != NULL);
    // We align down because of writes that are shorter than 4 B
    // As long as all writes are aligned, and 32 bits at max, this works
    bool hit = cpu->reserved_addr == ALIGN_DOWN(phys, 4);
    if(hit) {
        cpu->reserved_valid = false;
    }
    return hit;
}    




/* Interrupts
 * This is supposed to be used with devices and interprocessor communication,
 * devices should raise a Machine/Supervisor External Interrupt,
 * but interprocessor interrupts should be Machine/Supervisor Software Interrupts.
 * So we use the argument no to differentiate based on the exception code (see rv_exc_t definiton)
 */

void rv_interrupt_up(rv_cpu_t *cpu, unsigned int no){
    ASSERT(cpu != NULL);

    // default to MEI if no is invalid
    if( no != rv_exc_supervisor_software_interrupt &&
        no != rv_exc_machine_software_interrupt &&
        no != rv_exc_supervisor_external_interrupt &&
        no != rv_exc_machine_external_interrupt
    ){
        no = rv_exc_machine_external_interrupt;
    }

    uint32_t mask = RV_EXCEPTION_MASK(no);

    cpu->csr.mip |= mask;
}

void rv_interrupt_down(rv_cpu_t *cpu, unsigned int no){
    ASSERT(cpu != NULL);
    //! for simplicity just clears the bit
    //! if this interrupt could be raised by different means,
    //! this would not work!
    
    // default to MEI if no is invalid
    if( no != rv_exc_supervisor_software_interrupt &&
        no != rv_exc_machine_software_interrupt &&
        no != rv_exc_supervisor_external_interrupt &&
        no != rv_exc_machine_external_interrupt
    ){
        no = rv_exc_machine_external_interrupt;
    }

    uint32_t mask = RV_EXCEPTION_MASK(no);

    cpu->csr.mip &= ~mask;
}