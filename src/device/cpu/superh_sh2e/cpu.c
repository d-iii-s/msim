/*
 * Copyright (c) X-Y Z
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
#include "bitops.h"
#include "cpu.h"
#include "debug.h"
#include "decode.h"
#include "memops.h"


/** @brief Page of cached decoded instructions. */

typedef struct {
    item_t item;
    ptr36_t addr;
    sh2e_insn_exec_fn_t insns[FRAME_SIZE / sizeof(sh2e_insn_t)];
} cache_item_t;


#define PHYS2CACHEINSTR(phys) (((phys) & FRAME_MASK) / sizeof(sh2e_insn_t))

static void
cache_item_init(cache_item_t * cache_item) {
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
sh2e_cpu_insn_cache_get(ptr36_t phys, cache_item_t ** const output_item) {
    ASSERT(output_item != NULL);

    ptr36_t const target_page = ALIGN_DOWN(phys, FRAME_SIZE);
    printf("looking up insn cache page 0x%08" PRIx64 "\n", target_page);

    cache_item_t * item;
    for_each(sh2e_insn_cache, item, cache_item_t) {
        if (item->addr == target_page) {
            *output_item = item;
            return true;
        }
    }

    printf("  insn cache page not found\n");
    return false;
}


/**
 * @brief Converts a virtual address to physical address.
 *
 * @return `true` if the conversion succeeded, `false` otherwise.
 */
bool
sh2e_cpu_convert_addr(
    sh2e_cpu_t const * const restrict cpu,
    ptr64_t const virt, ptr36_t * const phys, bool const write
) {
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
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    sh2e_insn_t * const restrict output_insn
) {
    ASSERT(cpu != NULL);
    ASSERT(output_insn != NULL);

    // TODO Check alignment.
    output_insn->word = sh2e_physmem_read16(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}


/**
 * @brief Sets the PC to the given address.
 */
static void
sh2e_set_pc(sh2e_cpu_t * const restrict cpu, uint32_t const addr) {
    cpu->cpu_regs.pc = addr;
    cpu->pc_next = addr + sizeof(sh2e_insn_t);
}


#define address(ptr) (uintptr_t)(ptr)

/** Sets initial values of registers. */
static void
sh2e_power_on_reset(sh2e_cpu_t * const restrict cpu) {
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

    sh2e_eva_t * epva = (sh2e_eva_t *) address(cpu->cpu_regs.vbr);

    uint32_t eva_sp_addr = (uint32_t) address(&epva->power_on_reset_sp);
    cpu->cpu_regs.sp = sh2e_physmem_read32(cpu->id, eva_sp_addr, false);

    uint32_t eva_pc_addr = (uint32_t) address(&epva->power_on_reset_pc);
    cpu->cpu_regs.pc = sh2e_physmem_read32(cpu->id, eva_pc_addr, false);

    cpu->pr_state = SH2E_PSTATE_POWER_ON_RESET;
}


/****************************************************************************
 * Instruction decode cache
 ****************************************************************************/

static void
sh2e_cpu_insn_cache_decode_page(sh2e_cpu_t * const restrict cpu, cache_item_t * cache_item) {
    printf("decoding page 0x%08" PRIx64 "\n", cache_item->addr);

    for (size_t i = 0; i < FRAME_SIZE / sizeof(sh2e_insn_t); i++) {
        ptr36_t addr = cache_item->addr + (i * sizeof(sh2e_insn_t));
        sh2e_insn_t insn = (sh2e_insn_t) sh2e_physmem_read16(cpu->id, addr, false);
        sh2e_insn_desc_t const * desc = sh2e_insn_decode(insn);
        cache_item->insns[i] = desc->exec;
    }
}

static void
sh2e_cpu_insn_cache_update(sh2e_cpu_t * const restrict cpu, cache_item_t * cache_item) {
    frame_t * frame = physmem_find_frame(cache_item->addr);
    ASSERT(frame != NULL);

    if (!frame->valid) {
        sh2e_cpu_insn_cache_decode_page(cpu, cache_item);
        frame->valid = true;
    }

    return;
}

static cache_item_t *
sh2e_cpu_insn_cache_try_add(sh2e_cpu_t * const restrict cpu, ptr36_t phys) {
    printf("trying to add insn cache page for 0x%08" PRIx64 "\n", phys);
    frame_t * frame = physmem_find_frame(phys);
    if (frame == NULL) {
        return NULL;
    }

    printf("  found frame %p\n", frame);
    cache_item_t * cache_item = safe_malloc(sizeof(cache_item_t));
    cache_item_init(cache_item);
    cache_item->addr = ALIGN_DOWN(phys, FRAME_SIZE);
    list_push(&sh2e_insn_cache, &cache_item->item);

    sh2e_cpu_insn_cache_decode_page(cpu, cache_item);
    frame->valid = true;
    return cache_item;
}


static sh2e_insn_exec_fn_t
sh2e_cpu_fetch_insn_func(sh2e_cpu_t * const restrict cpu, ptr36_t phys) {
    cache_item_t * cache_item = NULL;
    if (sh2e_cpu_insn_cache_get(phys, &cache_item)) {
        ASSERT(cache_item != NULL);

        sh2e_cpu_insn_cache_update(cpu, cache_item);

        // Move the instruction cache page to the head of the list
        // (if not already there) to speed up subsequent accesses.
        if (sh2e_insn_cache.head != &cache_item->item) {
            list_remove(&sh2e_insn_cache, &cache_item->item);
            list_push(&sh2e_insn_cache, &cache_item->item);
        }

        // Return the function representing the instruction.
        return cache_item->insns[PHYS2CACHEINSTR(phys)];
    }

    //
    // There is no instruction cache page for the given address.
    // Add it and return the instruction.
    //
    cache_item = sh2e_cpu_insn_cache_try_add(cpu, phys);
    if (cache_item != NULL) {
        return cache_item->insns[PHYS2CACHEINSTR(phys)];
    }

    alert("Trying to fetch instructions from outside of physical memory");
    return NULL;
}


/**
 * @brief Execute the instruction that the PC is pointing at
 * and handle interrupts or exceptions.
 */
static sh2e_exception_t
sh2e_cpu_execute_insn(sh2e_cpu_t * const restrict cpu) {

    ptr36_t insn_phys = cpu->cpu_regs.pc;
    printf("executing insn at %p\n", (void *) insn_phys);

    sh2e_insn_t insn;
    sh2e_exception_t fetch_ex = sh2e_cpu_fetch_insn(cpu, insn_phys, &insn);
    if (fetch_ex != SH2E_EXCEPTION_NONE) {
        // TODO jump to exception handler
        alert("exception %d when fetching instruction", fetch_ex);
        return fetch_ex;
    }

    if (machine_trace) {
        sh2e_cpu_dump_insn(cpu, cpu->cpu_regs.pc, insn);
    }

    printf("fetching insn at 0x%08" PRIx64 "\n", insn_phys);
    sh2e_insn_exec_fn_t exec_insn = sh2e_cpu_fetch_insn_func(cpu, insn_phys);

    printf("calling instruction function: %p\n", exec_insn);
    return exec_insn(cpu, insn);
}


/****************************************************************************
 * SH-2E CPU
 ****************************************************************************/

/** @brief Initialize the processor after power-on. */
void
sh2e_cpu_init(sh2e_cpu_t * const restrict cpu, unsigned int id) {
    ASSERT(cpu != NULL);

    memset(cpu, 0, sizeof(sh2e_cpu_t));
    cpu->id = id;

    // Requires 'id' to be set.
    sh2e_power_on_reset(cpu);

    // Execution state.
    cpu->pc_next = cpu->cpu_regs.pc + sizeof(sh2e_insn_t);
}


/** @brief Cleanup CPU structures. */
void
sh2e_cpu_done(sh2e_cpu_t * const restrict cpu) {
    ASSERT(cpu != NULL);

    // Clean the entire instruction cache.
    while (!is_empty(&sh2e_insn_cache)) {
        cache_item_t * cache_item = (cache_item_t *) (sh2e_insn_cache.head);
        list_remove(&sh2e_insn_cache, &cache_item->item);
        safe_free(cache_item);
    }
}


/** @brief Execute one instruction. */
void
sh2e_cpu_step(sh2e_cpu_t * const restrict cpu) {
    ASSERT(cpu != NULL);

    sh2e_exception_t ex = sh2e_cpu_execute_insn(cpu);
    if (ex != SH2E_EXCEPTION_NONE) {
        // TODO Handle exceptions.
    } else {
        // TODO Check for interrupts.
    }

    // TODO Perform accounting.

    // Update program counter (respect delay slots).

    switch (cpu->br_state) {
    case SH2E_BRANCH_STATE_DELAY: // Delay branch instruction.
        cpu->br_state = SH2E_BRANCH_STATE_EXECUTE;
        // Fall-through.
    case SH2E_BRANCH_STATE_NONE: // Advance PC to next instruction.
        cpu->cpu_regs.pc += sizeof(sh2e_insn_t);
        break;

    case SH2E_BRANCH_STATE_EXECUTE: // Execute delayed branch.
        cpu->cpu_regs.pc = cpu->pc_next;
        cpu->br_state = SH2E_BRANCH_STATE_NONE;
        break;

    default:
        ASSERT(false && "invalid CPU branch state");
    }
}


/** @brief Set the address of the next instruction to execute. */
void
sh2e_cpu_goto(sh2e_cpu_t * const restrict cpu, ptr64_t const addr) {
    ASSERT(cpu != NULL);

    if (IS_ALIGNED(addr.lo, sizeof(sh2e_insn_t))) {
        sh2e_set_pc(cpu, addr.lo);
    } else {
        ASSERT(false && "attempt to set PC to unaligned address");
    }
}


/** Assert the specified interrupt. */
void
sh2e_cpu_assert_interrupt(sh2e_cpu_t * const restrict cpu, unsigned int num) {
    ASSERT(cpu != NULL);
    // TODO assert the interrupt.
}


/** Deassert the specified interrupt */
void
sh2e_cpu_deassert_interrupt(sh2e_cpu_t * const restrict cpu, unsigned int num) {
    ASSERT(cpu != NULL);
    // TODO assert the interrupt.
}
