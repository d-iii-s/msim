/*
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV64IMA SV39 virtual memory translation
 *
 */

#ifndef RISCV_RV64IMA_VIRT_MEM_H_
#define RISCV_RV64IMA_VIRT_MEM_H_

#include "../../../main.h"
#include "../riscv_rv_ima/types.h"

struct rv64_cpu;
enum rv64_exc;

#define RV64_PAGESIZE 12 // we have 4KiB (= 2^12) pages
#define RV64_PTESIZE 8
#define RV64_MEGAPAGESIZE 21 // we have 2MiB (= 2^21) megapages
#define RV64_GIGAPAGESIZE 30 // we have 1GiB (= 2^30) gigapages

#define RV64_PAGEBYTES (1 << RV64_PAGESIZE)
#define RV64_MEGAPAGEBYTES (1 << RV64_MEGAPAGESIZE)
#define RV64_GIGAPAGEBYTES (1 << RV64_GIGAPAGESIZE)

typedef enum sv39_page_type {
    page,
    megapage,
    gigapage
} sv39_page_type_t;

/**
 * @brief Structure describing the Page Table Entry for SV39 virtual addressing
 */
typedef struct sv39_pte {
    unsigned int v : 1;
    unsigned int r : 1;
    unsigned int w : 1;
    unsigned int x : 1;
    unsigned int u : 1;
    unsigned int g : 1;
    unsigned int a : 1;
    unsigned int d : 1;
    unsigned int rsw : 2;
    unsigned long ppn : 44;
    unsigned int _reserved : 7;
    unsigned int pbmt : 2;
    unsigned int n : 1;
} sv39_pte_t;

// The permission bits, R, W, and X, indicate whether the page is readable,
// writable, and executable, respectively. When all three are zero, the PTE is
// a pointer to the next level of the page table; otherwise, it is a leaf PTE.
#define sv39_is_pte_leaf(pte) ((pte).r | (pte).w | (pte).x)
#define sv39_is_pte_valid(pte) ((pte).v && (!(pte).w || (pte).r))
#define sv39_pte_ppn0(pte) ((pte).ppn & 0x1FF)
#define sv39_pte_ppn1(pte) (((pte).ppn >> 9) & 0x1FF)
#define sv39_pte_ppn2(pte) ((pte).ppn >> 18)
/*
 * @brief Shifts the PPN in a PTE by the pagesize (useful for preparing
 * a physical address).
 */
#define sv39_pte_ppn_phys(pte) (((ptr36_t) (pte).ppn) << RV64_PAGESIZE)

/* Sv39 PTEs are 64 bits in size */
typedef union {
    sv39_pte_t pte;
    uint64_t val;
} sv39_pte_helper_t;

/*
 * @brief val must be a 64bit integer!
 */
#define sv39_pte_from_uint(val) (((sv39_pte_helper_t) (val)).pte)
#define sv39_uint_from_pte(pte) (((sv39_pte_helper_t) (pte)).val)

/**
 * @brief Constructs the resulting physical address based on the given virtual address and pte (and whether it is a megapage)
 */
ptr55_t sv39_make_phys_from_ppn(virt_t virt, sv39_pte_t pte, sv39_page_type_t page_type);

#endif // RISCV_RV32IMA_VIRT_MEM_H_
