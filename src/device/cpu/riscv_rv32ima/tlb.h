/*
 * Copyright (c) 2023 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V TLB
 *
 */
#ifndef RISCV_RV32IMA_TLB_H_
#define RISCV_RV32IMA_TLB_H_

#include <stdint.h>
#include <stdbool.h>
#include "../../../main.h"
#include "virt_mem.h"

struct rv_tlb_entry;

typedef struct rv_tlb {
    struct rv_tlb_entry* ktlb; // Kilo-TLB for 4K size pages
    struct rv_tlb_entry* mtlb; // Mega-TLB for 4M size pages
    size_t ktlb_size;
    size_t mtlb_size; 
} rv_tlb_t;

#define DEFAULT_RV_KTLB_SIZE 256
#define DEFAULT_RV_MTLB_SIZE 32

/** Caches a mapping into the TLB */
extern void rv_tlb_add_mapping(rv_tlb_t* tlb, unsigned asid, uint32_t virt, sv32_pte_t pte, bool megapage, bool global);

/** Retrieves a cached mapping, giving priority to megapage mappings */
extern bool rv_tlb_get_mapping(rv_tlb_t* tlb, unsigned asid, uint32_t virt, sv32_pte_t* pte, bool* megapage);

/** TLB flushes */
extern void rv_tlb_flush(rv_tlb_t* tlb);
extern void rv_tlb_flush_by_asid(rv_tlb_t* tlb, unsigned asid);
extern void rv_tlb_flush_by_addr(rv_tlb_t* tlb, uint32_t virt);
extern void rv_tlb_flush_by_asid_and_addr(rv_tlb_t* tlb, unsigned asid, uint32_t virt);

/** Initializes the TLB data structure */
extern void rv_tlb_init(rv_tlb_t* tlb, size_t ktlb_size, size_t mtlb_size);
/** Cleans up the TLB structure */
extern void rv_tlb_done(rv_tlb_t* tlb);

/** Resizes the TLB
 *  Also flushes the TLB
 */
extern bool rv_tlb_resize_ktlb(rv_tlb_t* tlb, size_t size);
extern bool rv_tlb_resize_mtlb(rv_tlb_t* tlb, size_t size);

extern void rv_tlb_dump(rv_tlb_t* tlb);

#endif // RISCV_RV32IMA_TLB_H_
