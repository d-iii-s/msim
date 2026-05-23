/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../assert.h"
#include "../../dsh2ecmt.h"
#include "../../dsh2ewdt.h"
#include "../../intc/superh_sh2e/intc.h"
#include "../../peripheral.h"
#include "bitops.h"
#include "cpu.h"
#include "debug.h"
#include "decode.h"
#include "memops.h"

/** @brief Cached instruction item */
typedef struct {
    sh2e_insn_exec_fn_t insn;
    unsigned int cycles;
    bool disable_interrupts;
    bool disable_address_errors;
} cached_insn_t;

/** @brief Page of cached decoded instructions. */

typedef struct {
    item_t item;
    ptr36_t addr;
    cached_insn_t insns[FRAME_SIZE / sizeof(sh2e_insn_t)];
} cache_item_t;

#define PHYS2CACHEINSTR(phys) (((phys) & FRAME_MASK) / sizeof(sh2e_insn_t))

static void
cache_item_init(cache_item_t *cache_item)
{
    ASSERT(cache_item != NULL);

    item_init(&cache_item->item);
    cache_item->addr = 0;
    // Skip insns init.
}

static list_t sh2e_insn_cache = LIST_INITIALIZER;

/**
 * @brief Retrieves instruction cache page for the given address.
 *
 * @return `true` if a page exists, `false` otherwise.
 */
static bool
sh2e_cpu_insn_cache_get(ptr36_t phys, cache_item_t **const output_item)
{
    ASSERT(output_item != NULL);

    ptr36_t const target_page = ALIGN_DOWN(phys, FRAME_SIZE);

    cache_item_t *item;
    for_each(sh2e_insn_cache, item, cache_item_t)
    {
        if (item->addr == target_page) {
            *output_item = item;
            return true;
        }
    }
    return false;
}

/**
 * @brief Converts a virtual address to physical address.
 *
 * @return `true` if the conversion succeeded, `false` otherwise.
 */
bool sh2e_cpu_convert_addr(
        sh2e_cpu_t const *const restrict cpu,
        ptr64_t const virt, ptr36_t *const phys, bool const write)
{
    ASSERT(cpu != NULL);

    // Use identity, because SH-2E does not support memory mapping.
    *phys = (ptr36_t) virt.ptr;
    return true;
}

/**
 * @brief Reads an instruction from memory and stores it
 * into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param output_insn Pointer to where to store the instruction.
 * @return sh2e_exception_t Exception code related to the access.
 */
sh2e_exception_t
sh2e_cpu_fetch_insn(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        sh2e_insn_t *const restrict output_insn)
{
    ASSERT(cpu != NULL);
    ASSERT(output_insn != NULL);

    // Check alignment (must be even address).
    if (addr & 1) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    output_insn->word = sh2e_physmem_read16(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Sets the PC to the given address.
 */
static void
sh2e_set_pc(sh2e_cpu_t *const restrict cpu, uint32_t const addr)
{
    cpu->cpu_regs.pc = addr;
    cpu->pc_next = addr + sizeof(sh2e_insn_t);
}

#define address(ptr) (uintptr_t) (ptr)

/****************************************************************************
 * Instruction decode cache
 ****************************************************************************/

static void
sh2e_cpu_insn_cache_decode_page(sh2e_cpu_t *const restrict cpu, cache_item_t *cache_item)
{
    for (size_t i = 0; i < FRAME_SIZE / sizeof(sh2e_insn_t); i++) {
        ptr36_t addr = cache_item->addr + (i * sizeof(sh2e_insn_t));
        sh2e_insn_t insn = (sh2e_insn_t) sh2e_physmem_read16(cpu->id, addr, false);
        sh2e_insn_desc_t const *desc = sh2e_insn_decode(insn);
        cache_item->insns[i].insn = desc->exec;
        cache_item->insns[i].disable_interrupts = desc->disable_interrupts;
        cache_item->insns[i].disable_address_errors = desc->disable_address_errors;
        cache_item->insns[i].cycles = desc->cycles;
    }
}

static void
sh2e_cpu_insn_cache_update(sh2e_cpu_t *const restrict cpu, cache_item_t *cache_item)
{
    frame_t *frame = physmem_find_frame(cache_item->addr);
    ASSERT(frame != NULL);

    if (!frame->valid) {
        sh2e_cpu_insn_cache_decode_page(cpu, cache_item);
        frame->valid = true;
    }

    return;
}

static cache_item_t *
sh2e_cpu_insn_cache_try_add(sh2e_cpu_t *const restrict cpu, ptr36_t phys)
{
    frame_t *frame = physmem_find_frame(phys);
    if (frame == NULL) {
        return NULL;
    }

    cache_item_t *cache_item = safe_malloc(sizeof(cache_item_t));
    cache_item_init(cache_item);
    cache_item->addr = ALIGN_DOWN(phys, FRAME_SIZE);
    list_push(&sh2e_insn_cache, &cache_item->item);

    sh2e_cpu_insn_cache_decode_page(cpu, cache_item);
    frame->valid = true;
    return cache_item;
}

static sh2e_insn_exec_fn_t
sh2e_cpu_fetch_insn_func(sh2e_cpu_t *const restrict cpu, ptr36_t phys, unsigned int *insn_cycles)
{
    cache_item_t *cache_item = NULL;
    if (sh2e_cpu_insn_cache_get(phys, &cache_item)) {
        ASSERT(cache_item != NULL);

        sh2e_cpu_insn_cache_update(cpu, cache_item);

        // Move the instruction cache page to the head of the list
        // (if not already there) to speed up subsequent accesses.
        if (sh2e_insn_cache.head != &cache_item->item) {
            list_remove(&sh2e_insn_cache, &cache_item->item);
            list_push(&sh2e_insn_cache, &cache_item->item);
        }

        cached_insn_t *item = &cache_item->insns[PHYS2CACHEINSTR(phys)];

        cpu->disable_interrupts = item->disable_interrupts;
        cpu->disable_address_errors = item->disable_address_errors;
        *insn_cycles = item->cycles;

        // Return the function representing the instruction.
        return item->insn;
    }

    //
    // There is no instruction cache page for the given address.
    // Add it and return the instruction.
    //
    cache_item = sh2e_cpu_insn_cache_try_add(cpu, phys);
    if (cache_item != NULL) {

        cached_insn_t *item = &cache_item->insns[PHYS2CACHEINSTR(phys)];

        cpu->disable_interrupts = item->disable_interrupts;
        cpu->disable_address_errors = item->disable_address_errors;
        *insn_cycles = item->cycles;

        return item->insn;
    }

    alert("Trying to fetch instructions from outside of physical memory");
    return NULL;
}

/**
 * @brief Execute the instruction that the PC is pointing at
 * and handle interrupts or exceptions.
 */
static sh2e_exception_t
sh2e_cpu_execute_insn(sh2e_cpu_t *const restrict cpu, unsigned int *insn_cycles)
{

    ptr36_t insn_phys = cpu->cpu_regs.pc;

    sh2e_insn_t insn;
    sh2e_exception_t fetch_ex = sh2e_cpu_fetch_insn(cpu, insn_phys, &insn);
    if (fetch_ex != SH2E_EXCEPTION_NONE) {
        return fetch_ex;
    }

    if (machine_trace) {
        sh2e_cpu_dump_insn(cpu, cpu->cpu_regs.pc, insn);
    }

    sh2e_insn_exec_fn_t exec_insn = sh2e_cpu_fetch_insn_func(cpu, insn_phys, insn_cycles);

    return exec_insn(cpu, insn);
}

static void
sh2e_cpu_handle_exception(sh2e_cpu_t *const restrict cpu, sh2e_exception_t ex, bool disable_stacking_exception)
{
    cpu->pr_state = SH2E_PSTATE_EXCEPTION_PROCESSING;
    uint32_t vector_addr = 0;
    sh2e_eva_t *epva = (sh2e_eva_t *) address(cpu->cpu_regs.vbr);

    switch (ex) {
    case SH2E_EXCEPTION_ILLEGAL_INSTRUCTION: {
        vector_addr = (uint32_t) address(&epva->general_illegal_insn);
        break;
    }
    case SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION: {
        vector_addr = (uint32_t) address(&epva->slot_illegal_insn);
        break;
    }
    case SH2E_EXCEPTION_CPU_ADDRESS_ERROR: {
        vector_addr = (uint32_t) address(&epva->cpu_address_error);
        break;
    }
    case SH2E_EXCEPTION_FPU_OPERATION: {
        vector_addr = (uint32_t) address(&epva->fpu_exception);
        break;
    }
    default: {
        ASSERT(false && "Unhandled exception");
    }
    }

    if (machine_trace) {
        sh2e_cpu_dump_exception_state_data(cpu, ex, vector_addr);
    }

    uint32_t const stack_sr = cpu->cpu_regs.sr.value;
    uint32_t const stack_sr_addr = cpu->cpu_regs.sp - sizeof(uint32_t);

    uint32_t stack_pc = 0;
    switch (ex) {
    case SH2E_EXCEPTION_ILLEGAL_INSTRUCTION: {
        stack_pc = cpu->cpu_regs.pc;
    }
    case SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION:
    case SH2E_EXCEPTION_FPU_OPERATION:
    case SH2E_EXCEPTION_CPU_ADDRESS_ERROR: {
        stack_pc = cpu->pc_next;
    }
    default: {
        break;
        alert("Unsupported exception");
    }
    }

    uint32_t const stack_pc_addr = stack_sr_addr - sizeof(uint32_t);

    bool stack_ex = false;

    // Push SR to stack
    if (sh2e_cpu_write_long(cpu, stack_sr_addr, stack_sr) != SH2E_EXCEPTION_NONE) {
        stack_ex = true;
    }

    // Push PC to stack
    if (sh2e_cpu_write_long(cpu, stack_pc_addr, stack_pc) != SH2E_EXCEPTION_NONE) {
        stack_ex = true;
    }

    cpu->cpu_regs.sp = stack_pc_addr;
    cpu->cpu_regs.pc = sh2e_physmem_read32(cpu->id, vector_addr, false);
    cpu->pc_next = cpu->cpu_regs.pc + sizeof(sh2e_insn_t);

    // Handle stacking exception immediately (if not disabled)
    if (stack_ex && !disable_stacking_exception) {
        sh2e_cpu_handle_exception(cpu, SH2E_EXCEPTION_CPU_ADDRESS_ERROR, true);
    }

    // Clear the exception
    cpu->insn_exception = SH2E_EXCEPTION_NONE;

    // Go back to program execution state
    cpu->pr_state = SH2E_PSTATE_PROGRAM_EXECUTION;
}

static void
sh2e_cpu_handle_interrupt(sh2e_cpu_t *const restrict cpu)
{
    cpu->pr_state = SH2E_PSTATE_EXCEPTION_PROCESSING;

    // Send the interrupt signal to on-chip peripherals (if any) so that they can update their state accordingly.
    peripheral_t *peripheral;
    for_each(cpu->on_chip_peripherals, peripheral, peripheral_t)
    {
        peripheral->type->interrupt_up(peripheral, cpu->pending_interrupt);
    }

    uint32_t const stack_sr = cpu->cpu_regs.sr.value;
    uint32_t const stack_sr_addr = cpu->cpu_regs.sp - sizeof(uint32_t);

    uint32_t const stack_pc = cpu->pc_next;
    uint32_t const stack_pc_addr = stack_sr_addr - sizeof(uint32_t);

    bool stack_ex = false;

    // Push SR to stack
    if (sh2e_cpu_write_long(cpu, stack_sr_addr, stack_sr) != SH2E_EXCEPTION_NONE) {
        stack_ex = true;
    }

    // Push PC to stack (Address of instruction after executed instruction)
    if (sh2e_cpu_write_long(cpu, stack_pc_addr, stack_pc) != SH2E_EXCEPTION_NONE) {
        stack_ex = true;
    }

    sh2e_eva_t *epva = (sh2e_eva_t *) address(cpu->cpu_regs.vbr);

    cpu->cpu_regs.sp = stack_pc_addr;

    uint32_t eva_pc_addr = (uint32_t) address(&epva->vectors[cpu->pending_interrupt]);
    cpu->cpu_regs.pc = sh2e_physmem_read32(cpu->id, eva_pc_addr, false);
    cpu->pc_next = cpu->cpu_regs.pc + sizeof(sh2e_insn_t);

    uint32_t new_mask = 0;

    intc_accept_interrupt(cpu->intc, &new_mask);

    if (machine_trace) {
        sh2e_cpu_dump_interrupt_state_data(cpu, cpu->pending_interrupt, new_mask);
    }

    cpu->cpu_regs.sr.im = new_mask;

    // Handle stacking exception immediately
    if (stack_ex) {
        sh2e_cpu_handle_exception(cpu, SH2E_EXCEPTION_CPU_ADDRESS_ERROR, true);
    }

    // Clear the pending interrupt
    cpu->pending_interrupt = 0;
    // Go back to program execution state
    cpu->pr_state = SH2E_PSTATE_PROGRAM_EXECUTION;
}

static bool
sh2e_cpu_check_reset_req(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    uint8_t reset = 0;
    bool reset_pending = intc_check_resets(cpu->intc, &reset);

    if (!reset_pending) {
        return false;
    }

    switch (reset) {
    case SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET: // Power-on reset (external)
        cpu->reset_req = SH2E_POWER_ON_RESET_REQ_EXTERNAL;
        break;
    case SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET: // Power-on reset (internal)
        cpu->reset_req = SH2E_POWER_ON_RESET_REQ_INTERNAL;
        break;
    case SH2E_INTC_MANUAL_RESET_OFFSET: // Manual reset
        cpu->reset_req = SH2E_MANUAL_RESET_REQ;
        break;
    default:
        alert("Unknown reset request %u", reset);
    }

    cpu->pr_state = SH2E_PSTATE_RESET;

    return true;
}

static void
sh2e_cpu_reset(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    // Registers with defined values.

    cpu->cpu_regs.vbr = 0x00000000;

    cpu->cpu_regs.sr.value = 0;
    cpu->cpu_regs.sr.im = 0b1111;

    cpu->fpu_regs.fpscr.value = 0;
    cpu->fpu_regs.fpscr.dn = 1;
    cpu->fpu_regs.fpscr.rm = 0b01;

    // Registers with derived values.
    // Ignore exceptions when loading register values from EPVA.

    uint32_t eva_sp_addr = 0;
    uint32_t eva_pc_addr = 0;

    sh2e_eva_t *epva = (sh2e_eva_t *) address(cpu->cpu_regs.vbr);

    switch (cpu->reset_req) {
    case SH2E_POWER_ON_RESET_REQ_INTERNAL:
    case SH2E_POWER_ON_RESET_REQ_EXTERNAL:
    case SH2E_POWER_ON_RESET_INITIAL:
        eva_pc_addr = (uint32_t) address(&epva->power_on_reset_pc);
        eva_sp_addr = (uint32_t) address(&epva->power_on_reset_sp);
        break;
    case SH2E_MANUAL_RESET_REQ:
        eva_pc_addr = (uint32_t) address(&epva->manual_reset_pc);
        eva_sp_addr = (uint32_t) address(&epva->manual_reset_sp);
        break;
    default:
        alert("Unknown reset request type %d", cpu->reset_req);
    }

    cpu->cpu_regs.sp = sh2e_physmem_read32(cpu->id, eva_sp_addr, false);
    cpu->cpu_regs.pc = sh2e_physmem_read32(cpu->id, eva_pc_addr, false);

    cpu->pc_next = cpu->cpu_regs.pc + sizeof(sh2e_insn_t);

    // Initialize INTC (only if this is not the initial power-on reset because we want to keep the values from config).
    if (cpu->reset_req != SH2E_POWER_ON_RESET_INITIAL) {
        intc_init(cpu->intc);
    }
}

/****************************************************************************
 * CPU state steps
 ****************************************************************************/

/**
 * @brief Handle one step in the standby state.
 */
static void
sh2e_standby_state_step(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    // Check for reset requests.
    if (sh2e_cpu_check_reset_req(cpu)) {
        return;
    }

    uint8_t interrupt_source = 0;

    bool interrupt_pending = intc_check_interrupts(cpu->intc, cpu->cpu_regs.sr.im, &interrupt_source);

    if (machine_trace) {
        sh2e_cpu_dump_power_down_state_data(cpu, interrupt_pending, interrupt_source);
    }

    if (interrupt_pending) {
        cpu->pending_interrupt = interrupt_source;
        cpu->pr_state = SH2E_PSTATE_EXCEPTION_PROCESSING;
    }

    // Accounting
    cpu->power_down_cycles++;
}

/**
 * @brief Handle one step in the reset state.
 *
 * @param cpu The CPU instance.
 */
static void
sh2e_reset_state_step(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    if (machine_trace) {
        sh2e_cpu_dump_reset_state_data(cpu, cpu->reset_req);
    }

    intc_accept_reset(cpu->intc);

    // Initialize CPU/FPU and INTC
    sh2e_cpu_reset(cpu);

    peripheral_t *peripheral;
    bool raise_interrupt = false;
    unsigned int int_no = 0;

    switch (cpu->reset_req) {
    case SH2E_POWER_ON_RESET_REQ_INTERNAL: {
        int_no = SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET;
        raise_interrupt = true;
        break;
    }
    case SH2E_POWER_ON_RESET_REQ_EXTERNAL: {
        int_no = SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET;
        raise_interrupt = true;
        break;
    }
    case SH2E_POWER_ON_RESET_INITIAL: {
        // Nothing for now
        break;
    }
    case SH2E_MANUAL_RESET_REQ: {
        int_no = SH2E_INTC_MANUAL_RESET_OFFSET;
        raise_interrupt = true;
        // Nothing for now
        break;
    }
    case SH2E_RESET_REQ_NONE: {
        // Should not happen
        break;
    }
    }

    if (raise_interrupt) {
        // Signal the reset interrupt to on-chip peripherals
        for_each(cpu->on_chip_peripherals, peripheral, peripheral_t)
        {
            peripheral->type->interrupt_up(peripheral, int_no);
        }
    }

    // Clear the request
    cpu->reset_req = SH2E_RESET_REQ_NONE;
    cpu->pr_state = SH2E_PSTATE_PROGRAM_EXECUTION;
}

/**
 * @brief Handle one step in the exception processing state.
 */
static void
sh2e_exception_state_step(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    // Check if address errors are disabled.
    if (cpu->disable_address_errors && cpu->insn_exception == SH2E_EXCEPTION_CPU_ADDRESS_ERROR) {
        cpu->pending_address_error = true;
        cpu->insn_exception = SH2E_EXCEPTION_NONE;
    }

    // Exception Processing Priority Order:
    // - CPU address error
    if ((cpu->pending_address_error || cpu->insn_exception == SH2E_EXCEPTION_CPU_ADDRESS_ERROR) && !cpu->disable_address_errors) {
        cpu->pending_address_error = false;
        sh2e_cpu_handle_exception(cpu, SH2E_EXCEPTION_CPU_ADDRESS_ERROR, false);
        return;
    }

    // - FPU exception
    if (cpu->insn_exception == SH2E_EXCEPTION_FPU_OPERATION) {
        sh2e_cpu_handle_exception(cpu, cpu->insn_exception, false);
        return;
    }

    // - Interrupts
    if (cpu->pending_interrupt) {
        sh2e_cpu_handle_interrupt(cpu);
        return;
    }

    // - General illegal instructions
    // - Illegal slot instructions
    if (cpu->insn_exception != SH2E_EXCEPTION_NONE) {
        sh2e_cpu_handle_exception(cpu, cpu->insn_exception, false);
    }
}

/**
 * @brief Handle one step in the program execution state.
 */
static void
sh2e_program_execution_state_step(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    // Check for reset requests.
    if (sh2e_cpu_check_reset_req(cpu)) {
        return;
    }

    unsigned int insn_cycles = 0;

    if (cpu->pr_state != SH2E_PSTATE_POWER_DOWN) {
        cpu->insn_exception = sh2e_cpu_execute_insn(cpu, &insn_cycles);
    }

    // Can happen after executing the SLEEP instruction
    if (cpu->pr_state == SH2E_PSTATE_POWER_DOWN) {
        peripheral_t *peripheral;
        for_each(cpu->on_chip_peripherals, peripheral, peripheral_t)
        {
            // Using sleep mode for now, because we don't support the other standby modes yet.
            peripheral->type->interrupt_up(peripheral, SH2E_INTC_SLEEP_MODE_OFFSET);
        }
    }

    // If interrupts are not disabled, check for pending interrupts.
    if (!cpu->disable_interrupts) {

        uint8_t interrupt_source = 0;
        bool interrupt_pending = intc_check_interrupts(cpu->intc, cpu->cpu_regs.sr.im, &interrupt_source);

        if (interrupt_pending) {
            cpu->pending_interrupt = interrupt_source;
        }
    }

    // Go to exception processing state if there is an exception or a pending interrupt.
    if (cpu->insn_exception != SH2E_EXCEPTION_NONE || cpu->pending_address_error || cpu->pending_interrupt) {
        cpu->pr_state = SH2E_PSTATE_EXCEPTION_PROCESSING;
    }

    // Accounting
    cpu->program_execution_cycles += insn_cycles;
    peripheral_t *peripheral;
    for_each(cpu->on_chip_peripherals, peripheral, peripheral_t)
    {
        peripheral->type->update_cycles(peripheral, insn_cycles);
    }

    // Update program counter (respect delay slots).
    if (cpu->pr_state == SH2E_PSTATE_PROGRAM_EXECUTION) {
        switch (cpu->br_state) {
        case SH2E_BRANCH_STATE_DELAY_NEXT: { // Just executed a branch instruction, we change the state to delay and advance the PC to the next instruction, but we keep the jump address in pc_next for the delayed slot instruction.
            cpu->br_state = SH2E_BRANCH_STATE_DELAY;
            // We want to leave the value of the jump in the pc_next, so we can use it after the execution of the instruction in the delayed slot.
            // Therefore, we only advance the PC to the next instruction here.
            cpu->cpu_regs.pc += sizeof(sh2e_insn_t);
            break;
        }
        case SH2E_BRANCH_STATE_NONE: { // Advance PC to next instruction.
            cpu->cpu_regs.pc = cpu->pc_next;
            cpu->pc_next += sizeof(sh2e_insn_t);
            break;
        }
        case SH2E_BRANCH_STATE_DELAY: // Delay branch instruction.
        case SH2E_BRANCH_STATE_EXECUTE: { // Execute delayed branch.
            cpu->cpu_regs.pc = cpu->pc_next;
            cpu->pc_next += sizeof(sh2e_insn_t);
            cpu->br_state = SH2E_BRANCH_STATE_NONE;
            break;
        }
        default: {
            ASSERT(false && "invalid CPU branch state");
        }
        }
    }
}

/****************************************************************************
 * SH-2E CPU
 ****************************************************************************/

/** @brief Initialize the processor after power-on. */
void sh2e_cpu_init(sh2e_cpu_t *const restrict cpu, unsigned int id)
{
    ASSERT(cpu != NULL);

    memset(cpu, 0, sizeof(sh2e_cpu_t));
    cpu->id = id;

    cpu->pr_state = SH2E_PSTATE_RESET;
    cpu->reset_req = SH2E_POWER_ON_RESET_INITIAL;

    cpu->on_chip_peripherals = (list_t) LIST_INITIALIZER;
}

/** @brief Cleanup CPU structures. */
void sh2e_cpu_done(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    // Clean the entire instruction cache.
    while (!is_empty(&sh2e_insn_cache)) {
        cache_item_t *cache_item = (cache_item_t *) (sh2e_insn_cache.head);
        list_remove(&sh2e_insn_cache, &cache_item->item);
        safe_free(cache_item);
    }
}

/** @brief Execute the next cpu step depending on the current cpu state. */
void sh2e_cpu_step(sh2e_cpu_t *const restrict cpu)
{
    ASSERT(cpu != NULL);

    switch (cpu->pr_state) {
    case SH2E_PSTATE_RESET:
        sh2e_reset_state_step(cpu);
        break;
    case SH2E_PSTATE_EXCEPTION_PROCESSING:
        sh2e_exception_state_step(cpu);
        break;
    case SH2E_PSTATE_PROGRAM_EXECUTION:
        sh2e_program_execution_state_step(cpu);
        break;
    case SH2E_PSTATE_POWER_DOWN:
        sh2e_standby_state_step(cpu);
        break;
    case SH2E_PSTATE_BUS_RELEASE:
        // Nothing
        break;

    default:
        ASSERT(false && "invalid CPU processing state");
    }
}

/** @brief Set the address of the next instruction to execute. */
void sh2e_cpu_goto(sh2e_cpu_t *const restrict cpu, ptr64_t const addr)
{
    ASSERT(cpu != NULL);

    if (IS_ALIGNED(addr.lo, sizeof(sh2e_insn_t))) {
        sh2e_set_pc(cpu, addr.lo);
    } else {
        ASSERT(false && "attempt to set PC to unaligned address");
    }
}

/** Assert the specified interrupt. */
void sh2e_cpu_assert_interrupt(sh2e_cpu_t *const restrict cpu, unsigned int num)
{
    intc_interrupt_up(cpu->intc, num);
}

/** Deassert the specified interrupt */
void sh2e_cpu_deassert_interrupt(sh2e_cpu_t *const restrict cpu, unsigned int num)
{
    intc_interrupt_down(cpu->intc, num);
}
