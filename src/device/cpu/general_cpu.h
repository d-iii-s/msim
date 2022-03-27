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

#ifndef GENERAL_CPU_H_
#define GENERAL_CPU_H_

#include <stdbool.h>
#include "../../main.h"
#include "../../debug/breakpoint.h"

typedef void (*interrupt_func_t)(void*, unsigned int);
typedef void (*insert_breakpoint_func_t)(void*, ptr64_t, breakpoint_t);
typedef void (*remove_breakpoint_func_t)(void*, ptr64_t);
typedef bool (*convert_addr_func_t)(void*, ptr64_t, ptr36_t*, bool);
typedef void (*reg_dump_func_t)(void*);
typedef void (*set_pc_func_t)(void*, ptr64_t);
typedef bool (*sc_access_func_t)(void*, ptr36_t);

typedef struct {
    interrupt_func_t interrupt_up;
    interrupt_func_t interrupt_down;
    insert_breakpoint_func_t insert_breakpoint;
    remove_breakpoint_func_t remove_breakpoint;
    convert_addr_func_t convert_addr;
    reg_dump_func_t reg_dump;
    set_pc_func_t set_pc;
    sc_access_func_t sc_access;
} cpu_ops_t;

typedef struct {
    item_t item;
    unsigned int cpuno;
    const cpu_ops_t *type;
    void *data;
} general_cpu_t;


extern general_cpu_t* get_cpu(unsigned int no);

extern unsigned int get_free_cpuno(void);

extern void add_cpu(general_cpu_t *cpu);
extern void remove_cpu(general_cpu_t *cpu);

extern void cpu_interrupt_up(general_cpu_t *cpu, unsigned int no);
extern void cpu_interrupt_down(general_cpu_t *cpu, unsigned int no);

extern void cpu_insert_breakpoint(general_cpu_t *cpu, ptr64_t addr, breakpoint_t kind);
extern void cpu_remove_breakpoint(general_cpu_t *cpu, ptr64_t addr);

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
extern bool cpu_convert_addr(general_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write);

extern void cpu_reg_dump(general_cpu_t *cpu);

// todo: register read/write for gdb

extern void cpu_set_pc(general_cpu_t *cpu, ptr64_t pc);
/**
 * @brief signals to the cpu, that an address has been written to, for sc control
 * 
 * @param cpu the processor pointer
 * @param addr the address that is written to
 * @return whether the address was linked/reserved
 */
extern bool cpu_sc_access(general_cpu_t *cpu, ptr36_t addr);

#endif // GENERAL_CPU_H_