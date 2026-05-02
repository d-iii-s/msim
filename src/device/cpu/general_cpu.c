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
#include "../../utils.h"
#include "general_cpu.h"

list_t cpu_list = LIST_INITIALIZER;

general_cpu_t *general_cpu_init(const unsigned int id, void *cpu, const cpu_ops_t *type)
{
    general_cpu_t *gen_cpu = safe_malloc_t(general_cpu_t);
    gen_cpu->cpuno = id;
    gen_cpu->data = cpu;
    gen_cpu->type = type;
    list_init(&gen_cpu->bps);
    return gen_cpu;
}

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

bool cpu_normalize_bp_addr(general_cpu_t *cpu, const ptr64_t addr, ptr64_t *out)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->normalize_bp_addr(cpu->data, addr, out);
}

bool cpu_insert_breakpoint(general_cpu_t *cpu, const ptr64_t addr, const breakpoint_kind_t kind)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }

    ptr64_t normalized_addr = { 0 };
    if (!cpu->type->normalize_bp_addr(cpu->data, addr, &normalized_addr)) {
        if (kind == BREAKPOINT_KIND_SIMULATOR) {
            error("Virtual address out of range");
        }
        return false;
    }

    const breakpoint_t *existing = breakpoint_find_by_address(cpu->bps, normalized_addr, (breakpoint_filter_t) kind);
    if (existing == NULL) {
        breakpoint_t *bp = breakpoint_init(normalized_addr, kind);
        list_append(&cpu->bps, &bp->item);
    }
    // Only report error for simulator breakpoints, debuggers are left to handle it as they want
    else if (kind == BREAKPOINT_KIND_SIMULATOR) {
        error("Breakpoint already exists");
    }
    return true;
}

bool cpu_remove_breakpoint(general_cpu_t *cpu, const ptr64_t addr, const breakpoint_kind_t kind)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }

    ptr64_t normalized_addr = { 0 };
    if (!cpu->type->normalize_bp_addr(cpu->data, addr, &normalized_addr)) {
        if (kind == BREAKPOINT_KIND_SIMULATOR) {
            error("Virtual address out of range");
        }
        return false;
    }

    breakpoint_t *remove = breakpoint_find_by_address(cpu->bps, normalized_addr, (breakpoint_filter_t) kind);
    if (remove != NULL) {
        list_remove(&cpu->bps, &remove->item);
        safe_free(remove);
    }
    // Only report error for simulator breakpoints, debuggers are left to handle it as they want
    else if (kind == BREAKPOINT_KIND_SIMULATOR) {
        error("Unknown breakpoint");
    }
    return true;
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

bool cpu_get_reg(general_cpu_t *cpu, unsigned int regno, uint64_t *out_value)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->get_reg(cpu->data, regno, out_value);
}

bool cpu_set_reg(general_cpu_t *cpu, unsigned int regno, uint64_t value)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->set_reg(cpu->data, regno, value);
}

bool cpu_get_csr(general_cpu_t *cpu, unsigned int regno, uint64_t *out_value)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->get_csr(cpu->data, regno, out_value);
}

bool cpu_set_csr(general_cpu_t *cpu, unsigned int regno, uint64_t value)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->set_csr(cpu->data, regno, value);
}

ptr64_t cpu_get_pc(general_cpu_t *cpu)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->get_pc(cpu->data);
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

cpu_arch_t cpu_get_arch(general_cpu_t *cpu)
{
    if (cpu == NULL) {
        cpu = get_fallback_cpu();
    }
    return cpu->type->arch;
}
