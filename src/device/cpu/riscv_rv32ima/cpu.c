/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV32IMA simulation
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../assert.h"
#include "../../../list.h"
#include "../../../main.h"
#include "../../../physmem.h"
#include "../../../utils.h"
#include "cpu.h"
#include "csr.h"
#include "tlb.h"
#include "virt_mem.h"

/** Generic types */
#include "../riscv_rv_ima/exception.h"
#include "../riscv_rv_ima/instr.h"

/// Caching of decoded instructions

/**
 * @brief Item of a list of pages of cached instructions
 */
typedef struct {
    item_t item; // The list item
    ptr36_t addr; // The base address of the page
    rv_instr_func_t instrs[FRAME_SIZE / sizeof(rv_instr_t)]; // Decoded instructions (represented as function pointers)
} cache_item_t;

#define PHYS2CACHEINSTR(phys) (((phys) & FRAME_MASK) / sizeof(rv_instr_t))

static void cache_item_init(cache_item_t *cache_item)
{
    item_init(&cache_item->item);
    cache_item->addr = 0;
    // Skip instrs init
}

list_t rv_instruction_cache = LIST_INITIALIZER;

/**
 * @brief Looks into cache for the instruction on the given physical address
 * @par phys The physical address to check
 * @par cache_item A pointer to a found cache_item_t will be stored here on success
 * @returns Whether the address is in the cache
 */
static bool cache_hit(ptr36_t phys, cache_item_t **cache_item)
{
    ptr36_t target_page = ALIGN_DOWN(phys, FRAME_SIZE);
    cache_item_t *c;
    for_each(rv_instruction_cache, c, cache_item_t)
    {
        if (c->addr == target_page) {
            *cache_item = c;
            return true;
        }
    }
    return false;
}

static void init_regs(rv32_cpu_t *cpu)
{
    ASSERT(cpu != NULL);

    // expects that default value for any variable is 0

    cpu->pc = RV_START_ADDRESS;
    cpu->pc_next = RV_START_ADDRESS + 4;
}

/**
 * @brief Initialize the CPU
 */
void rv32_cpu_init(rv32_cpu_t *cpu, unsigned int procno)
{

    ASSERT(cpu != NULL);

    memset(cpu, 0, sizeof(rv32_cpu_t));

    init_regs(cpu);

    rv32_init_csr(&cpu->csr, procno);

    rv32_tlb_init(&cpu->tlb, DEFAULT_RV_TLB_SIZE);

    cpu->priv_mode = rv_mmode;
}

/**
 * @brief Cleanup CPU structures
 */
void rv32_cpu_done(rv32_cpu_t *cpu)
{
    // Clean whole cache for simplicity whenever any cpu is done
    while (!is_empty(&rv_instruction_cache)) {
        cache_item_t *cache_item = (cache_item_t *) (rv_instruction_cache.head);
        list_remove(&rv_instruction_cache, &cache_item->item);
        safe_free(cache_item);
    }

    rv32_tlb_done(&cpu->tlb);
}

static_assert((sizeof(sv32_pte_t) == 4), "wrong size of sv32_pte_t");

/**
 * @brief Tests, whether a given memory access is allowed on the page specified by the pte
 *
 * @par wr True for Write, False for Read
 * @par fetch whether the access is an instruction fetch
 */
static bool is_access_allowed(rv32_cpu_t *cpu, sv32_pte_t pte, bool wr, bool fetch)
{
    if (wr && !pte.w) {
        return false;
    }

    if (fetch && !pte.x) {
        return false;
    }

    // Page is executable and I can read from executable pages
    bool rx = rv_csr_sstatus_mxr(cpu) && pte.x;

    if (!wr && !fetch && !pte.r && !rx) {
        return false;
    }

    if (sv32_effective_priv(cpu) == rv_smode) {
        if (!rv_csr_sstatus_sum(cpu) && pte.u) {
            return false;
        }
        if (fetch && pte.u) {
            return false;
        }
    }

    if (sv32_effective_priv(cpu) == rv_umode) {
        if (!pte.u) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Constructs the resulting physical address based on the given virtual address and pte (and whether it is a megapage)
 */
ptr36_t make_phys_from_ppn(uint32_t virt, sv32_pte_t pte, bool megapage)
{
    ptr36_t page_offset = virt & 0x00000FFF;
    ptr36_t virt_vpn0 = virt & 0x003FF000;
    ptr36_t pte_ppn0 = (ptr36_t) pte_ppn0(pte) << 12;
    ptr36_t pte_ppn1 = (ptr36_t) pte_ppn1(pte) << 12;
    ptr36_t phys_ppn0 = megapage ? virt_vpn0 : pte_ppn0;

    return pte_ppn1 | phys_ppn0 | page_offset;
}

#define page_fault_exception (fetch ? rv_exc_instruction_page_fault : (wr ? rv_exc_store_amo_page_fault : rv_exc_load_page_fault))

static inline bool pte_access_dirty_update_needed(sv32_pte_t pte, bool wr)
{
    return pte.a == 0 || (pte.d == 0 && wr);
}

/**
 * @brief Tranlates the virtual address to physical by Sv32 memory translation algorithm, modifying the pagetable by doing so (setting the Accessed and Dirty bits and populating the TLB)
 *
 */
static rv_exc_t rv32_pagewalk(rv32_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy)
{
    uint32_t vpn0 = (virt & 0x003FF000) >> 12;
    uint32_t vpn1 = (virt & 0xFFC00000) >> 22;
    uint32_t ppn = rv_csr_satp_ppn(cpu);

    ptr36_t a = ((ptr36_t) ppn) << RV_PAGESIZE;
    ptr36_t pte_addr = a + vpn1 * RV_PTESIZE;

    // PMP or PMA check goes here if implemented
    uint32_t pte_val = physmem_read32(cpu->csr.mhartid, pte_addr, noisy);

    sv32_pte_t pte = pte_from_uint(pte_val);

    if (!is_pte_valid(pte)) {
        return page_fault_exception;
    }

    bool is_megapage = false;
    bool is_global = false;

    if (is_pte_leaf(pte)) {
        // MEGAPAGE
        // Missaligned megapage
        if (pte_ppn0(pte) != 0) {
            return page_fault_exception;
        }
        is_megapage = true;
        is_global = pte.g;
    } else {
        // Non leaf PTE, make second translation step

        // PMP or PMA check goes here if implemented
        a = pte_ppn_phys(pte);
        pte_addr = a + vpn0 * RV_PTESIZE;

        // Global non-leaf PTE implies that the translation is global
        is_global = pte.g;

        pte_val = physmem_read32(cpu->csr.mhartid, pte_addr, noisy);
        pte = pte_from_uint(pte_val);

        if (!is_pte_valid(pte)) {
            return page_fault_exception;
        }

        // Non-leaf on last level
        if (!is_pte_leaf(pte)) {
            return page_fault_exception;
        }

        // The translation is global if the non-leaf PTE is global or if the leaf PTE is global
        is_global |= pte.g;
    }

    if (!is_access_allowed(cpu, pte, wr, fetch)) {
        return page_fault_exception;
    }

    if (pte_access_dirty_update_needed(pte, wr)) {

        // No need to check with memory, we have just read it and this operation is done atomically

        pte.a = 1;
        pte.d |= wr ? 1 : 0;

        pte_val = uint_from_pte(pte);

        if (noisy) {
            physmem_write32(cpu->csr.mhartid, pte_addr, pte_val, true);
        }
    }

    *phys = make_phys_from_ppn(virt, pte, is_megapage);

    // Add the leaf PTE of the translation to the TLB
    rv32_tlb_add_mapping(&cpu->tlb, rv_csr_satp_asid(cpu), virt, pte, is_megapage, is_global);

    return rv_exc_none;
}

/**
 * @brief Converts the address from virtual memory space to physical memory space
 *
 * @param cpu The CPU, from the point of which, is the translation made
 * @param virt The virtual address to be converted
 * @param phys Pointer to where the physical address will be stored
 * @param wr Is the conversion made for a write operation
 * @param fetch Is the conversion made for an instruction fetch
 * @param noisy Shall this function change the processor and global state
 * @return rv_exc_t The exception code of this operation
 */
static rv_exc_t rv_convert_addr(rv_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy)
{
    // TODO: should pagewalk trigger memory breakpoints?

    ASSERT(cpu != NULL);
    ASSERT(phys != NULL);
    ASSERT(!(wr && fetch));

    bool use_sv32 = (sv32_effective_priv(cpu) <= rv_smode) && (!rv_csr_satp_is_bare(cpu)) && !(cpu->priv_mode == rv_mmode && fetch);

    if (!use_sv32) {
        *phys = virt;
        return rv_exc_none;
    }

    unsigned asid = rv_csr_satp_asid(cpu);
    sv32_pte_t pte;
    bool megapage;

    // First try the TLB
    if (rv32_tlb_get_mapping(&cpu->tlb, asid, virt, &pte, &megapage, true)) {

        if (!is_pte_valid(pte)) {
            return page_fault_exception;
        }

        // Check access rights of the cached pte
        if (!is_access_allowed(cpu, pte, wr, fetch)) {
            return page_fault_exception;
        }

        // Missaligned magapage
        if (megapage && pte_ppn0(pte) != 0) {
            return page_fault_exception;
        }

        // If the A and D bits of the PTE do not need to be updated, we can use the cached result
        if (!pte_access_dirty_update_needed(pte, wr)) {
            *phys = make_phys_from_ppn(virt, pte, megapage);
            return rv_exc_none;
        } else {
            // Flush stale entry from cache
            rv32_tlb_remove_mapping(&cpu->tlb, asid, virt);
        }
    }

    // If the TLB lookup failed or if the AD bits need to be updated, perform the full pagewalk
    return rv32_pagewalk(cpu, virt, phys, wr, fetch, noisy);
}
#undef page_fault_exception

rv_exc_t rv32_convert_addr(rv32_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy)
{
    return rv_convert_addr(cpu, virt, phys, wr, fetch, noisy);
}

/** First we include memory helpers */
#include "../riscv_rv_ima/memory.c"
/** Then instruction implementations */
#include "instr.c"

/**
 * @brief Fills the cache_item instrs field with decoded data based on the addr field
 */
static void cache_item_page_decode(rv32_cpu_t *cpu, cache_item_t *cache_item)
{
    for (size_t i = 0; i < FRAME_SIZE / sizeof(rv_instr_t); ++i) {
        ptr36_t addr = cache_item->addr + (i * sizeof(rv_instr_t));
        rv_instr_t instr_data = (rv_instr_t) physmem_read32(cpu->csr.mhartid, addr, false);
        cache_item->instrs[i] = rv32_instr_decode(instr_data);
    }
}

/**
 * @brief Updates the cache item to represent the data in memory
 */
static void update_cache_item(rv32_cpu_t *cpu, cache_item_t *cache_item)
{
    frame_t *frame = physmem_find_frame(cache_item->addr);
    ASSERT(frame != NULL);

    if (frame->valid) {
        return;
    }

    cache_item_page_decode(cpu, cache_item);

    frame->valid = true;
    return;
}

/**
 * @brief Tries to add a new entry to the cache based on the given address
 *
 * @return cache_item_t* the added intem or NULL, if the address does not lead to valid memory area
 */
static cache_item_t *cache_try_add(rv32_cpu_t *cpu, ptr36_t phys)
{
    frame_t *frame = physmem_find_frame(phys);
    if (frame == NULL) {
        return NULL;
    }

    cache_item_t *cache_item = safe_malloc(sizeof(cache_item_t));

    cache_item_init(cache_item);
    cache_item->addr = ALIGN_DOWN(phys, FRAME_SIZE);

    list_push(&rv_instruction_cache, &cache_item->item);

    cache_item_page_decode(cpu, cache_item);

    frame->valid = true;
    return cache_item;
}

/**
 * @brief Fethes a decoded instruction from memory
 *
 * Consults the cache first and updates the cache on misses or on invalid memory
 */
static rv_instr_func_t fetch_instr(rv32_cpu_t *cpu, ptr36_t phys)
{
    cache_item_t *cache_item = NULL;

    if (cache_hit(phys, &cache_item)) {
        ASSERT(cache_item != NULL);
        update_cache_item(cpu, cache_item);

        // Move item to front of list on hit

        if (rv_instruction_cache.head != &cache_item->item) {
            list_remove(&rv_instruction_cache, &cache_item->item);
            list_push(&rv_instruction_cache, &cache_item->item);
        }

        return cache_item->instrs[PHYS2CACHEINSTR(phys)];
    }

    cache_item = cache_try_add(cpu, phys);

    if (cache_item != NULL) {
        return cache_item->instrs[PHYS2CACHEINSTR(phys)];
    }
    alert("Trying to fetch instructions from outside of physical memory");
    return rv32_instr_decode((rv_instr_t) physmem_read32(cpu->csr.mhartid, phys, true));
}

/**
 * @brief Sets the PC to the given virtual address
 *
 */
void rv32_cpu_set_pc(rv_cpu_t *cpu, uint32_t value)
{
    ASSERT(cpu != NULL);
    if (!IS_ALIGNED(value, 4)) {
        return;
    }
    /* Set both pc and pc_next
     * This should be called from the debugger to jump somewhere
     * and in case the new instruction does not modify pc_next,
     * the processor would then jump back to where it was before this call
     */
    cpu->pc = value;
    cpu->pc_next = value + 4;
}

/**
 * @brief Trap to M mode
 *
 * @param ex The exception/interrupt that caused the trap
 */
static void m_trap(rv_cpu_t *cpu, rv_exc_t ex)
{
    ASSERT(ex != rv_exc_none);

    bool is_interrupt = ex & RV_INTERRUPT_EXC_BITS;
    cpu->stdby = false;

    cpu->csr.mepc = is_interrupt ? cpu->pc_next : cpu->pc;
    cpu->csr.mcause = ex;
    cpu->csr.mtval = cpu->csr.tval_next;

    // MPIE = MIE
    {
        bool mie_set = rv_csr_mstatus_mie(cpu);

        if (mie_set) {
            cpu->csr.mstatus |= rv_csr_mstatus_mpie_mask;
        } else {
            cpu->csr.mstatus &= ~rv_csr_mstatus_mpie_mask;
        }
    }
    // MIE = 0
    {
        cpu->csr.mstatus &= ~rv_csr_mstatus_mie_mask;
    }
    // MPP = cpu->priv_mode
    {
        cpu->csr.mstatus &= ~rv_csr_mstatus_mpp_mask;
        cpu->csr.mstatus |= ((uint32_t) cpu->priv_mode << rv_csr_mstatus_mpp_pos) & rv_csr_mstatus_mpp_mask;
    }

    cpu->priv_mode = rv_mmode;

    int mode = cpu->csr.mtvec & rv_csr_mtvec_mode_mask;
    uint32_t base = cpu->csr.mtvec & ~rv_csr_mtvec_mode_mask;

    if (mode == rv_csr_mtvec_mode_direct) {
        cpu->pc_next = base;
    } else if (mode == rv_csr_mtvec_mode_vectored) {
        if (is_interrupt) {
            cpu->pc_next = base + 4 * (ex & ~RV_INTERRUPT_EXC_BITS);
        } else {
            cpu->pc_next = base;
        }
    } else {
        ASSERT(false);
    }
}

/**
 * @brief Trap to S mode
 *
 * @param ex The exception/interrupt that caused the trap
 */
static void s_trap(rv_cpu_t *cpu, rv_exc_t ex)
{
    ASSERT(ex != rv_exc_none);

    bool is_interrupt = ex & RV_INTERRUPT_EXC_BITS;
    cpu->stdby = false;

    cpu->csr.sepc = is_interrupt ? cpu->pc_next : cpu->pc;
    cpu->csr.scause = ex;
    cpu->csr.stval = cpu->csr.tval_next;

    // SPIE = SIE
    {
        bool sie_set = rv_csr_sstatus_sie(cpu);

        if (sie_set) {
            cpu->csr.mstatus |= rv_csr_sstatus_spie_mask;
        } else {
            cpu->csr.mstatus &= ~rv_csr_sstatus_spie_mask;
        }
    }
    // SIE = 0
    {
        cpu->csr.mstatus &= ~rv_csr_sstatus_sie_mask;
    }
    // SPP = cpu->priv_mode
    {
        cpu->csr.mstatus &= ~rv_csr_sstatus_spp_mask;
        cpu->csr.mstatus |= ((uint32_t) cpu->priv_mode << rv_csr_sstatus_spp_pos) & rv_csr_sstatus_spp_mask;
    }

    cpu->priv_mode = rv_smode;

    int mode = cpu->csr.stvec & rv_csr_mtvec_mode_mask;
    uint32_t base = cpu->csr.stvec & ~rv_csr_mtvec_mode_mask;

    if (mode == rv_csr_mtvec_mode_direct) {
        cpu->pc_next = base;
    } else if (mode == rv_csr_mtvec_mode_vectored) {
        if (is_interrupt) {
            cpu->pc_next = base + 4 * (ex & ~RV_INTERRUPT_EXC_BITS);
        } else {
            cpu->pc_next = base;
        }
    } else {
        ASSERT(false);
    }
}

/**
 * @brief Causes an exception trap to the proper privilege level
 */
static void handle_exception(rv_cpu_t *cpu, rv_exc_t ex)
{
    ASSERT(!(ex & RV_INTERRUPT_EXC_BITS));

    uint32_t mask = RV_EXCEPTION_MASK(ex);
    bool delegated = cpu->csr.medeleg & mask;

    if (delegated && cpu->priv_mode != rv_mmode) {
        s_trap(cpu, ex);
    } else {
        m_trap(cpu, ex);
    }
}

/**
 * @brief Traps if an interrupt is pending and is enabled
 *
 * Respects the proper interrupt priorities
 *
 */
static void try_handle_interrupt(rv_cpu_t *cpu)
{

    // Effective mip includes the external SEIP
    // Full explanation RISC-V Privileged spec section 3.1.9 Machine Interrupt Registers (mip and mie)
    uint32_t mip = cpu->csr.mip
            | (cpu->csr.external_SEIP ? rv_csr_sei_mask : 0)
            | (cpu->csr.external_STIP ? rv_csr_sti_mask : 0);

    // no interrupt pending
    if (mip == 0) {
        return;
    }

// PRIORITY: MEI, MSI, MTI, SEI, SSI, STI
#define trap_if_set(cpu, mask, interrupt, trap_func) \
    if (mask & RV_EXCEPTION_MASK(interrupt)) { \
        trap_func(cpu, interrupt); \
        return; \
    }

    // TRAP to M-mode
    // ((priv_mode == M && MIE) || (priv_mode < M)) && MIP[i] && MIE[i] && !MIDELEG[i]

    bool can_trap_to_M = (cpu->priv_mode == rv_mmode && rv_csr_mstatus_mie(cpu)) || (cpu->priv_mode < rv_mmode);

    if (can_trap_to_M) {
        uint32_t m_mode_active_interrupt_mask = mip & cpu->csr.mie & ~cpu->csr.mideleg;

        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_machine_external_interrupt, m_trap);
        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_machine_software_interrupt, m_trap);
        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_machine_timer_interrupt, m_trap);
        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_supervisor_external_interrupt, m_trap);
        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_supervisor_software_interrupt, m_trap);
        trap_if_set(cpu, m_mode_active_interrupt_mask, rv_exc_supervisor_timer_interrupt, m_trap);
    }

    // TRAP to S-mode
    // ((priv_mode == S && SIE) || (priv_mode < M)) && SIP[i] && SIE[i]

    bool can_trap_to_S = (cpu->priv_mode == rv_smode && rv_csr_sstatus_sie(cpu)) || (cpu->priv_mode < rv_smode);
    if (can_trap_to_S) {
        // mask to only account S mode interrupts
        uint32_t s_mode_active_interrupt_mask = mip & cpu->csr.mie & rv_csr_si_mask;

        // M-interrupts can be here theoretically by spec, but we don't allow the delegation of M interrupts in msim (which is allowed in spec)
        trap_if_set(cpu, s_mode_active_interrupt_mask, rv_exc_supervisor_external_interrupt, s_trap);
        trap_if_set(cpu, s_mode_active_interrupt_mask, rv_exc_supervisor_software_interrupt, s_trap);
        trap_if_set(cpu, s_mode_active_interrupt_mask, rv_exc_supervisor_timer_interrupt, s_trap);
    }

#undef trap_if_set
}

/**
 * @brief Increases the HPM counters based in the event specifying CSRs
 *
 * @param cpu The cpu on which these counters are
 * @param i The index of the HPM in range [0..29)
 */
static void account_hmp(rv_cpu_t *cpu, int i)
{
    ASSERT((i >= 0 && i < 29));

    uint32_t mask = (1 << (i + 3));
    bool inhibited = cpu->csr.mcountinhibit & mask;

    if (inhibited) {
        return;
    }

    rv_csr_hpm_event_t event = cpu->csr.hpmevents[i];

    switch (event) {
    case (hpm_u_cycles): {
        if (cpu->priv_mode == rv_umode) {
            cpu->csr.hpmcounters[i]++;
        }
        break;
    }
    case (hpm_s_cycles): {
        if (cpu->priv_mode == rv_smode) {
            cpu->csr.hpmcounters[i]++;
        }
        break;
    }
    case (hpm_m_cycles): {
        if (cpu->priv_mode == rv_mmode) {
            cpu->csr.hpmcounters[i]++;
        }
        break;
    }
    case (hpm_w_cycles): {
        if (cpu->stdby) {
            cpu->csr.hpmcounters[i]++;
        }
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Raises and clears timer interrupts based on the content of the coresponding CSRs
 *
 */
static void manage_timer_interrupts(rv32_cpu_t *cpu)
{
    // raise or clear scyclecmp ESTIP
    cpu->csr.external_STIP = ((uint32_t) cpu->csr.cycle) >= cpu->csr.scyclecmp;

    // raise or clear mtimecmp MTIP
    handle_mtip(cpu);
}

/**
 * @brief Increase the counter CSRs and raise timer interrupts if desired
 */
static void account(rv32_cpu_t *cpu, bool instruction_retired)
{
    if (!(cpu->csr.mcountinhibit & 0b001)) {
        cpu->csr.cycle++;
    }

    // mtime cannot be inhibited
    uint64_t current_tick_time = current_timestamp();
    cpu->csr.mtime += (current_tick_time - cpu->csr.last_tick_time);
    cpu->csr.last_tick_time = current_tick_time;

    if (!(cpu->csr.mcountinhibit & 0b100) && instruction_retired) {
        cpu->csr.instret++;
    }

    for (int i = 0; i < 29; ++i) {
        account_hmp(cpu, i);
    }

    manage_timer_interrupts(cpu);
}

/**
 * @brief Execute the instruction that PC is pointing to and handle interrupts or exceptions
 */
static rv_exc_t execute(rv32_cpu_t *cpu)
{
    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, cpu->pc, &phys, false, true, true);

    if (ex != rv_exc_none) {
        alert("Fetching from unconvertable address!");
        if (machine_trace) {
            // rv32_idump(cpu, cpu->pc, (rv_instr_t) 0U);
        }
        return ex;
    }

    rv_instr_func_t instr_func = fetch_instr(cpu, phys);
    rv_instr_t instr_data = (rv_instr_t) physmem_read32(cpu->csr.mhartid, phys, true);

    if (machine_trace) {
        rv32_idump(cpu, cpu->pc, instr_data);
    }

    ex = instr_func(cpu, instr_data);

    if (ex == rv_exc_illegal_instruction) {
        cpu->csr.tval_next = instr_data.val;
    }

    return ex;
}

/**
 * @brief Simulate one step of the CPU
 */
void rv32_cpu_step(rv32_cpu_t *cpu)
{
    ASSERT(cpu != NULL);

    rv_exc_t ex = rv_exc_none;
    bool instruction_retired = false;

    if (!cpu->stdby) {
        ex = execute(cpu);
        instruction_retired = (ex == rv_exc_none);
    }

    if (ex != rv_exc_none) {
        handle_exception(cpu, ex);
    } else {
        // If any interrupts are pending, handle them
        try_handle_interrupt(cpu);
    }

    account(cpu, instruction_retired);

    if (!cpu->stdby) {
        cpu->pc = cpu->pc_next;
        cpu->pc_next = cpu->pc + 4;
    }

    // x0 is always 0
    cpu->regs[0] = 0;
    cpu->csr.tval_next = 0;
}

/**
 * @brief Notify the CPU that an adress has been writen ti
 *
 * Used for implementing the LR/SC atomics
 *
 * @returns Whether we hit the LR reserved address
 */
bool rv32_sc_access(rv32_cpu_t *cpu, ptr36_t phys, int size)
{
    return rv_sc_access(cpu, phys, size);
}

/* Interrupts
 * This is supposed to be used with devices and interprocessor communication,
 * devices should raise a Machine/Supervisor External Interrupt,
 * but interprocessor interrupts should be Machine/Supervisor Software Interrupts.
 * So we use the argument `no` to differentiate based on the exception code (see `rv_exc_t` definiton)
 */

/**
 * @brief Raises the interrupt of the given number
 *
 * @param no The interrupt number (1 = SSI, 3 = MSI, 5 = STI, 7 = MTI, 9 = SEI, 11 = MEI)
 */
void rv32_interrupt_up(rv32_cpu_t *cpu, unsigned int no)
{
    ASSERT(cpu != NULL);

    // Edge case, where we don't want to set SEIP, because SEIP is writable from M mode
    // Full explanation RISC-V Privileged spec section 3.1.9 Machine Interrupt Registers (mip and mie)
    if (no == RV_INTERRUPT_NO(rv_exc_supervisor_external_interrupt)) {
        cpu->csr.external_SEIP = true;
        return;
    }

    // default to MEI if no is invalid
    if (no != RV_INTERRUPT_NO(rv_exc_machine_software_interrupt) && no != RV_INTERRUPT_NO(rv_exc_supervisor_software_interrupt) && no != RV_INTERRUPT_NO(rv_exc_machine_external_interrupt)) {
        no = RV_INTERRUPT_NO(rv_exc_machine_external_interrupt);
    }

    uint32_t mask = RV_EXCEPTION_MASK(no);

    cpu->csr.mip |= mask;
}

/**
 * @brief Clears the interrupt of the given number
 *
 * @param no The interrupt number (1 = SSI, 3 = MSI, 5 = STI, 7 = MTI, 9 = SEI, 11 = MEI)
 */
void rv32_interrupt_down(rv32_cpu_t *cpu, unsigned int no)
{
    ASSERT(cpu != NULL);
    //! for simplicity just clears the bit
    //! if this interrupt could be raised by different means,
    //! this would not work!

    // Edge case, where we don't want to clear SEIP, because SEIP is writable from M mode
    // Full explanation RISC-V Privileged spec section 3.1.9 Machine Interrupt Registers (mip and mie)
    if (no == RV_INTERRUPT_NO(rv_exc_supervisor_external_interrupt)) {
        cpu->csr.external_SEIP = false;
        return;
    }

    // default to MEI if no is invalid
    if (no != RV_INTERRUPT_NO(rv_exc_machine_software_interrupt) && no != RV_INTERRUPT_NO(rv_exc_supervisor_software_interrupt) && no != RV_INTERRUPT_NO(rv_exc_machine_external_interrupt)) {
        no = RV_INTERRUPT_NO(rv_exc_machine_external_interrupt);
    }

    uint32_t mask = RV_EXCEPTION_MASK(no);

    cpu->csr.mip &= ~mask;
}
