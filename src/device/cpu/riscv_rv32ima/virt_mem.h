/*
 * Copyright (c) 2023 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV32IMA SV32 virtual memory translation
 *
 */

#ifndef RISCV_RV32IMA_VIRT_MEM_H_
#define RISCV_RV32IMA_VIRT_MEM_H_

#include "../../../main.h"

struct rv_cpu;
enum rv_exc;

#define RV_PAGESIZE 12
#define RV_PTESIZE 4
#define RV_MEGAPAGESIZE 22

/**
 * @brief Structure describing the Page Table Entry for SV32 virtual addressing
 */
typedef struct sv32_pte {
    unsigned int v : 1;
    unsigned int r : 1;
    unsigned int w : 1;
    unsigned int x : 1;
    unsigned int u : 1;
    unsigned int g : 1;
    unsigned int a : 1;
    unsigned int d : 1;
    unsigned int rsw : 2;
    unsigned int ppn : 22;
} sv32_pte_t;

#define is_pte_leaf(pte) ((pte).r | (pte).w | (pte).x)
#define is_pte_valid(pte) ((pte).v && (!(pte).w || (pte).r))
#define pte_ppn0(pte) ((pte).ppn & 0x0003FF)
#define pte_ppn1(pte) ((pte).ppn & 0x3FFC00)
#define pte_ppn_phys(pte)(((ptr36_t)(pte).ppn) << RV_PAGESIZE)

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
enum rv_exc rv_convert_addr(struct rv_cpu *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);

/**
 * @brief Constructs the resulting physical address based on the given virtual address and pte (and whether it is a megapage)
 */
ptr36_t make_phys_from_ppn(uint32_t virt, sv32_pte_t pte, bool megapage);

#endif // RISCV_RV32IMA_VIRT_MEM_H_
