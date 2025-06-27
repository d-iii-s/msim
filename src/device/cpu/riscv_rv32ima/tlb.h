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

#include <stdbool.h>
#include <stdint.h>

#define XLEN 32

#include "../../../list.h"
#include "../../../main.h"
#include "virt_mem.h"

struct rv32_tlb_entry;

typedef struct rv32_tlb {
    struct rv32_tlb_entry *entries;
    size_t size;
    list_t lru_list;
    list_t free_list;
} rv32_tlb_t;

#define DEFAULT_RV_TLB_SIZE 48

/** Caches a mapping into the TLB */
extern void rv32_tlb_add_mapping(rv32_tlb_t *tlb, unsigned asid, uint32_t virt, sv32_pte_t pte, bool megapage, bool global);

/** Retrieves a cached mapping, giving priority to megapage mappings */
extern bool rv32_tlb_get_mapping(rv32_tlb_t *tlb, unsigned asid, uint32_t virt, sv32_pte_t *pte, bool *megapage, bool noisy);

/** Removes the first mapping that matches the given address and is global or has the right ASID */
extern void rv32_tlb_remove_mapping(rv32_tlb_t *tlb, unsigned asid, uint32_t virt);

/** TLB flushes */
extern void rv32_tlb_flush(rv32_tlb_t *tlb);
extern void rv32_tlb_flush_by_asid(rv32_tlb_t *tlb, unsigned asid);
extern void rv32_tlb_flush_by_addr(rv32_tlb_t *tlb, uint32_t virt);
extern void rv32_tlb_flush_by_asid_and_addr(rv32_tlb_t *tlb, unsigned asid, uint32_t virt);

/** Initializes the TLB data structure */
extern void rv32_tlb_init(rv32_tlb_t *tlb, size_t size);
/** Cleans up the TLB structure */
extern void rv32_tlb_done(rv32_tlb_t *tlb);

/** Resizes the TLB
 *  Also flushes the TLB
 */
extern bool rv32_tlb_resize(rv32_tlb_t *tlb, size_t size);

extern void rv32_tlb_dump(rv32_tlb_t *tlb);

#endif // RISCV_RV32IMA_TLB_H_
