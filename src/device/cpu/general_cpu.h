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
/** Function type for normalizing breakpoint addresses */
typedef bool (*normalize_bp_addr_func_t)(void *, ptr64_t, ptr64_t *);
/** Function type for converting addresses */
typedef bool (*convert_addr_func_t)(void *, ptr64_t, ptr36_t *, bool);
/** Function type for dumping register content */
typedef void (*reg_dump_func_t)(void *);
/** Function type for getting the value of a general register of a cpu */
typedef bool (*get_reg_func_t)(void *, unsigned int, uint64_t *);
/** Function type for setting the value of a general register of a cpu */
typedef bool (*set_reg_func_t)(void *, unsigned int, uint64_t);
/** Function type for getting the program counter of a cpu */
typedef ptr64_t (*get_pc_func_t)(void *);
/** Function type for setting the program counter of a cpu */
typedef void (*set_pc_func_t)(void *, ptr64_t);
/** Function type for notifying the processor about a write to a memory location, used for implementing SC atomic*/
typedef bool (*sc_access_func_t)(void *, ptr36_t, int);

/** The supported CPU architectures */
typedef enum cpu_arch {
    /** MIPS R4000 architecture. */
    CpuArchMips,
    /** RISC-V 32-bit architecture. */
    CpuArchRiscV32,
    /** RISC-V 64-bit architecture. */
    CpuArchRiscV64,
} cpu_arch_t;

/** Cpu method table
 *
 * NULL value means "not implemented"
 */
typedef struct {
    interrupt_func_t interrupt_up; /** Raise an interrupt */
    interrupt_func_t interrupt_down; /** Cancel an interrupt */
    normalize_bp_addr_func_t normalize_bp_addr; /** Normalize a breakpoint address, e.g. to align it to instruction boundary or anything arch-specific */
    convert_addr_func_t convert_addr;
    reg_dump_func_t reg_dump;
    get_reg_func_t get_reg;
    set_reg_func_t set_reg;
    get_pc_func_t get_pc;
    set_pc_func_t set_pc;
    sc_access_func_t sc_access;
    cpu_arch_t arch; /** The architecture of the CPU */
} cpu_ops_t;

/** Structure describing general CPU */
typedef struct {
    item_t item;
    unsigned int cpuno;
    const cpu_ops_t *type;
    void *data;
    list_t bps; // Breakpoints
} general_cpu_t;

/** List of all CPUs */
extern list_t cpu_list;

/**
 * @brief Allocates and initializes a general_cpu_t structure
 *
 * @param id The CPU id
 * @param cpu A pointer to the CPU-specific data structure
 * @param type A pointer to the cpu_ops_t structure describing the CPU methods
 * @return A pointer to the initialized general_cpu_t structure
 */
extern general_cpu_t *general_cpu_init(unsigned int id, void *cpu, const cpu_ops_t *type);

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

/**
 * @brief Normalizes an address for breakpoint insertion
 *
 * This is used to align the address to instruction boundaries
 * or to do any other architecture-specific normalization
 * that is required for breakpoint insertion.
 *
 * @param cpu The cpu for which the address will be normalized
 * @param addr The address that will be normalized
 * @param out A pointer to the variable where the normalized address will be stored,
 *           only modified if the function returns true
 * @return true if the address was successfully normalized, false otherwise
 */
extern bool cpu_normalize_bp_addr(general_cpu_t *cpu, ptr64_t addr, ptr64_t *out);

/**
 * @brief Inserts a code breakpoint at the given address
 *
 * @param cpu The cpu on which the breakpoint will be inserted
 * @param addr The address at which the breakpoint will be inserted
 * @param kind The breakpoint kind
 * @return true if the breakpoint was successfully inserted, false otherwise
 */
extern bool cpu_insert_breakpoint(general_cpu_t *cpu, ptr64_t addr, breakpoint_kind_t kind);

/**
 * @brief Removes a code breakpoint at the given address
 *
 * @param cpu The cpu on which the breakpoint will be removed
 * @param addr The address at which the breakpoint will be removed
 * @param kind The breakpoint kind, only this kind of breakpoint will be removed
 * @return true if the breakpoint was successfully removed, false otherwise
 */
extern bool cpu_remove_breakpoint(general_cpu_t *cpu, ptr64_t addr, breakpoint_kind_t kind);

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

/**
 * @brief Gets the value of a general register of the cpu
 *
 * @param cpu the processor pointer
 * @param regno the index of the register to get
 * @param out_value a pointer to the variable where the register value will be stored, only modified if the function returns true
 * @return true if the register value was successfully retrieved, false otherwise
 */
extern bool cpu_get_reg(general_cpu_t *cpu, unsigned int regno, uint64_t *out_value);

/**
 * @brief Sets the value of a general register of the cpu
 *
 * @param cpu the processor pointer
 * @param regno the index of the register to set
 * @param value the value that will be set to the register
 * @return true if the register value was successfully set, false otherwise
 */
extern bool cpu_set_reg(general_cpu_t *cpu, unsigned int regno, uint64_t value);

extern ptr64_t cpu_get_pc(general_cpu_t *cpu);
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

/**
 * @brief Get the architecture of the CPU
 *
 * @param cpu the processor pointer
 * @return the architecture of the CPU
 */
extern cpu_arch_t cpu_get_arch(general_cpu_t *cpu);

#endif // GENERAL_CPU_H_
