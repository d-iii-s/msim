/*
 * Copyright (c) 2023 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V RV64 TLB
 *
 */
#ifndef RISCV_RV64IMA_TLB_H_
#define RISCV_RV64IMA_TLB_H_

#include <stdbool.h>
#include <stdint.h>

#include "../../../list.h"
#include "../../../main.h"
#include "virt_mem.h"

struct rv64_tlb_entry;

typedef struct rv64_tlb {
    struct rv64_tlb_entry *entries;
    size_t size;
    list_t lru_list;
    list_t free_list;
} rv64_tlb_t;

#define DEFAULT_RV64_TLB_SIZE 96

/** Caches a mapping into the TLB */
extern void rv64_tlb_add_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt, sv39_pte_t pte, sv39_page_type_t page_type, bool global);

/** Retrieves a cached mapping, giving priority to megapage mappings */
extern bool rv64_tlb_get_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt, sv39_pte_t *pte, sv39_page_type_t *page_type, bool noisy);

/** Removes the first mapping that matches the given address and is global or has the right ASID */
extern void rv64_tlb_remove_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt);

/** TLB flushes */
extern void rv64_tlb_flush(rv64_tlb_t *tlb);
extern void rv64_tlb_flush_by_asid(rv64_tlb_t *tlb, unsigned asid);
extern void rv64_tlb_flush_by_addr(rv64_tlb_t *tlb, uint64_t virt);
extern void rv64_tlb_flush_by_asid_and_addr(rv64_tlb_t *tlb, unsigned asid, uint64_t virt);

/** Initializes the TLB data structure */
extern void rv64_tlb_init(rv64_tlb_t *tlb, size_t size);
/** Cleans up the TLB structure */
extern void rv64_tlb_done(rv64_tlb_t *tlb);

/** Resizes the TLB
 *  Also flushes the TLB
 */
extern bool rv64_tlb_resize(rv64_tlb_t *tlb, size_t size);

extern void rv64_tlb_dump(rv64_tlb_t *tlb);

#endif // RISCV_RV32IMA_TLB_H_
