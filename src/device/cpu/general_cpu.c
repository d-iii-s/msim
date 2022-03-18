#include "general_cpu.h"
#include "../../main.h"

void cpu_interrupt_up(general_cpu_t *cpu, unsigned int no){
    cpu->type->interrupt_up(cpu->data, no);
}

void cpu_interrupt_down(general_cpu_t *cpu, unsigned int no){
    cpu->type->interrupt_down(cpu->data, no);
}

void cpu_insert_breakpoint(general_cpu_t *cpu, ptr64_t addr, breakpoint_t kind){
    cpu->type->insert_breakpoint(cpu->data, addr, kind);
}
void cpu_remove_breakpoint(general_cpu_t *cpu, ptr64_t addr){
    cpu->type->remove_breakpoint(cpu->data, addr);
}

/**
 * @brief converts an address from virtual to physical memory, not modifying cpu state
 * 
 * @param cpu the processor pointer
 * @param virt virtual address
 * @param phys physical return address
 * @param write is the access a write or a read
 * @return true the translation was successful
 * @return false the translation was unsuccessful
 */
bool cpu_convert_addr(general_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write){
    return cpu->type->convert_addr(cpu->data, virt, phys, write);
}

void cpu_reg_dump(general_cpu_t *cpu){
    cpu->type->reg_dump(cpu->data);
}

// todo: register read/write for gdb

void cpu_set_pc(general_cpu_t *cpu, ptr64_t pc){
    cpu->type->set_pc(cpu->data, pc);
}
/**
 * @brief signals to the cpu, that an address has been written to, for sc control
 * 
 * @param cpu the processor pointer
 * @param addr the address that is written to
 * @return whether the address was linked/reserved
 */
extern bool cpu_sc_access(general_cpu_t *cpu, ptr36_t addr){
    return cpu->type->sc_access(cpu->data, addr);
}