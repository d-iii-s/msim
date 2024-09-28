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

#include "../../debug/breakpoint.h"
#include "../../main.h"

/** Function type for raising and canceling interrupts */
typedef void (*interrupt_func_t)(void *, unsigned int);
/** Function type for inserting breakpoints */
typedef void (*insert_breakpoint_func_t)(void *, ptr64_t, breakpoint_t);
/** Function type for removing breakpoints */
typedef void (*remove_breakpoint_func_t)(void *, ptr64_t);
/** Function type for converting addresses */
typedef bool (*convert_addr_func_t)(void *, ptr64_t, ptr36_t *, bool);
/** Function type for dumping register content */
typedef void (*reg_dump_func_t)(void *);
/** Function type for setting the program counter of a cpu */
typedef void (*set_pc_func_t)(void *, ptr64_t);
/** Function type for notifying the processor about a write to a memory location, used for implementing SC atomic*/
typedef bool (*sc_access_func_t)(void *, ptr36_t, int);

/** Cpu method table
 *
 * NULL value means "not implemented"
 */
typedef struct {
    interrupt_func_t interrupt_up; /** Rainse an interrupt */
    interrupt_func_t interrupt_down; /** Cancel an interrupt */
    insert_breakpoint_func_t insert_breakpoint;
    remove_breakpoint_func_t remove_breakpoint;
    convert_addr_func_t convert_addr;
    reg_dump_func_t reg_dump;
    set_pc_func_t set_pc;
    sc_access_func_t sc_access;
} cpu_ops_t;

/** Structure describing CPU methods */
typedef struct {
    item_t item;
    unsigned int cpuno;
    const cpu_ops_t *type;
    void *data;
} general_cpu_t;

/**
 * @brief Retrieves the general_cpu_t structure based on the given cpu id
 */
extern general_cpu_t *get_cpu(unsigned int no);
/**
 * @brief Returns the lowest unused cpu id or MAX_CPUS if none are available
 */
extern unsigned int get_free_cpuno(void);

/**
 * @brief Adds the CPU to the list of all cpus
 */
extern void add_cpu(general_cpu_t *cpu);

/**
 * @brief Removes the CPU from the list of all cpus
 */
extern void remove_cpu(general_cpu_t *cpu);

/**
 * @brief Raises an interrupt
 *
 * @param cpu The cpu on which the interrupt will be raised
 * @param no The interrupt number that will be raised
 */
extern void cpu_interrupt_up(general_cpu_t *cpu, unsigned int no);
/**
 * @brief Cancels an interrupt
 *
 * @param cpu The cpu in which the interrupt will be canceled
 * @param no The interrupt number, that will be canceled
 */
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

/**
 * @brief Dumps the registers of the CPU to stdout in a cpu-specific format
 */
extern void cpu_reg_dump(general_cpu_t *cpu);

extern void cpu_set_pc(general_cpu_t *cpu, ptr64_t pc);

/**
 * @brief signals to the cpu, that an address has been written to, for sc control
 *
 * @param cpu the processor pointer
 * @param addr the address that is written to
 * @param size the width of the access
 * @return whether the address was linked/reserved
 */
extern bool cpu_sc_access(general_cpu_t *cpu, ptr36_t addr, int size);

#endif // GENERAL_CPU_H_
