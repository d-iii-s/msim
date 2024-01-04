/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  General interface for cpu devices
 *
 */

#include "../../assert.h"
#include "../../main.h"
#include "general_cpu.h"

// list of all cpus
list_t cpu_list = LIST_INITIALIZER;

general_cpu_t *get_cpu(unsigned int no)
{
    general_cpu_t *cpu;
    for_each(cpu_list, cpu, general_cpu_t)
    {
        if (cpu->cpuno == no) {
            return cpu;
        }
    }
    return NULL;
}

/**
 * @brief Get first available cpu number
 *
 * @return First available cpu number or MAX_CPUS if no more CPU slots are available
 */

unsigned int get_free_cpuno(void)
{
    unsigned int c;
    unsigned int id_mask = 0;
    general_cpu_t *cpu;

    for_each(cpu_list, cpu, general_cpu_t)
    {
        id_mask |= 1 << (cpu->cpuno);
    }

    for (c = 0; c < MAX_CPUS; c++, id_mask >>= 1) {
        if (!(id_mask & 1)) {
            return c;
        }
    }

    return MAX_CPUS;
}

void add_cpu(general_cpu_t *cpu)
{
    item_init(&cpu->item);
    list_append(&cpu_list, &cpu->item);
}

void remove_cpu(general_cpu_t *cpu)
{
    list_remove(&cpu_list, &cpu->item);
}

static general_cpu_t *get_fallback_cpu(void)
{
    general_cpu_t *cpu = get_cpu(0);
    if (cpu == NULL) {
        cpu = (general_cpu_t *) cpu_list.head;
    }
    ASSERT(cpu != NULL);
    return cpu;
}

void cpu_interrupt_up(general_cpu_t *cpu, unsigned int no)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    cpu->type->interrupt_up(cpu->data, no);
}

void cpu_interrupt_down(general_cpu_t *cpu, unsigned int no)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    cpu->type->interrupt_down(cpu->data, no);
}

void cpu_insert_breakpoint(general_cpu_t *cpu, ptr64_t addr, breakpoint_t kind)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    cpu->type->insert_breakpoint(cpu->data, addr, kind);
}
void cpu_remove_breakpoint(general_cpu_t *cpu, ptr64_t addr)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
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
bool cpu_convert_addr(general_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->convert_addr(cpu->data, virt, phys, write);
}

void cpu_reg_dump(general_cpu_t *cpu)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    cpu->type->reg_dump(cpu->data);
}

void cpu_set_pc(general_cpu_t *cpu, ptr64_t pc)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    cpu->type->set_pc(cpu->data, pc);
}
/**
 * @brief signals to the cpu, that an address has been written to, for sc control
 *
 * @param cpu the processor pointer
 * @param addr the address that is written to
 * @param size the width of the access
 * @return whether the address was linked/reserved
 */
extern bool cpu_sc_access(general_cpu_t *cpu, ptr36_t addr, int size)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->sc_access(cpu->data, addr, size);
}
