/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Memory utils
 *
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#include <stdbool.h>

#include "../../../assert.h"
#include "../../../physmem.h"
#include "../../../utils.h"
#include "csr.h"
#include "exception.h"
#include "types.h"

#if XLEN == 64
#define rv_convert_addr rv64_convert_addr
#elif XLEN == 32
#define rv_convert_addr rv32_convert_addr
#endif

#define read_address_misaligned_exception (fetch ? rv_exc_instruction_address_misaligned : rv_exc_load_address_misaligned)

#define try_read_memory_mapped_regs_body(cpu, virt, value, width, type) \
    if (!IS_ALIGNED(virt, width / 8)) \
        return false; \
    if (((cpu)->priv_mode < rv_mmode) || rv_csr_mstatus_mprv(cpu)) \
        return false; \
    int offset = (virt & 0x7) * 8; \
    if (ALIGN_DOWN(virt, 8) == RV_MTIME_ADDRESS) { \
        *value = (type) EXTRACT_BITS(cpu->csr.mtime, offset, offset + (width) / 8); \
        return true; \
    } \
    if (ALIGN_DOWN(virt, 8) == RV_MTIMECMP_ADDRESS) { \
        *value = (type) EXTRACT_BITS(cpu->csr.mtimecmp, offset, offset + (width) / 8); \
        return true; \
    } \
    return false;

/** @brief Reads from memory mapped registers if there are any located on the given address */
static bool try_read_memory_mapped_regs_64(rv_cpu_t *cpu, virt_t virt, uint64_t *value)
{
    try_read_memory_mapped_regs_body(cpu, virt, value, 64, uint64_t)
}

/** @brief Reads from memory mapped registers if there are any located on the given address */
static bool try_read_memory_mapped_regs_32(rv_cpu_t *cpu, virt_t virt, uint32_t *value)
{
    try_read_memory_mapped_regs_body(cpu, virt, value, 32, uint32_t)
}

/** @brief Reads from memory mapped registers if there are any located on the given address */
static bool try_read_memory_mapped_regs_16(rv_cpu_t *cpu, virt_t virt, uint16_t *value)
{
    try_read_memory_mapped_regs_body(cpu, virt, value, 16, uint16_t)
}

/** @brief Reads from memory mapped registers if there are any located on the given address */
static bool try_read_memory_mapped_regs_8(rv_cpu_t *cpu, virt_t virt, uint8_t *value)
{
    try_read_memory_mapped_regs_body(cpu, virt, value, 8, uint8_t)
}

#undef try_read_memory_mapped_regs_body

static void handle_mtip(rv_cpu_t *cpu)
{
    if (cpu->csr.mtime >= cpu->csr.mtimecmp) {
        // Set MTIP
        cpu->csr.mip |= rv_csr_mti_mask;
    } else {
        // Clear MTIP
        cpu->csr.mip &= ~rv_csr_mti_mask;
    }
}

/** @brief Writes to memory mapped registers if there are any located on the given address */
static bool try_write_memory_mapped_regs(rv_cpu_t *cpu, uint64_t virt, uint64_t value, int width)
{
    if (!IS_ALIGNED(virt, width / 8)) {
        return false;
    }

    if ((cpu->priv_mode < rv_mmode) || rv_csr_mstatus_mprv(cpu)) {
        return false;
    }
    int offset = (virt & 0x7) * 8;
    if (ALIGN_DOWN(virt, 8) == RV_MTIME_ADDRESS) {
        cpu->csr.mtime = WRITE_BITS(cpu->csr.mtime, value, offset, offset + width);
        handle_mtip(cpu);
        return true;
    }
    if (ALIGN_DOWN(virt, 8) == RV_MTIMECMP_ADDRESS) {
        cpu->csr.mtimecmp = WRITE_BITS(cpu->csr.mtimecmp, value, offset, offset + width);
        handle_mtip(cpu);
        return true;
    }
    return false;
}

#define throw_ex(cpu, virt, ex, noisy) \
    { \
        if (noisy) { \
            (cpu)->csr.tval_next = (virt); \
        } \
        return (ex); \
    }

/**
 * @brief Reads 64 bits from virtual memory
 *
 * @param cpu The cpu which makes the read
 * @param virt The virtual address of the read target
 * @param value The pointer where will the read value be stored on success
 * @param fetch If the read is instruction fetch or data read
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_read_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t *value, bool fetch, bool noisy)
{
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);

    if (try_read_memory_mapped_regs_64(cpu, virt, value)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, fetch, noisy);

    // Address translation exceptions have priority to alignment exceptions

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 8)) {
        throw_ex(cpu, virt, read_address_misaligned_exception, noisy);
    }

    *value = physmem_read64(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

/**
 * @brief Reads 32 bits from virtual memory
 *
 * @param cpu The cpu which makes the read
 * @param virt The virtual address of the read target
 * @param value The pointer where will the read value be stored on success
 * @param fetch If the read is instruction fetch or data read
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_read_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t *value, bool fetch, bool noisy)
{
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);

    if (try_read_memory_mapped_regs_32(cpu, virt, value)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, fetch, noisy);

    // Address translation exceptions have priority to alignment exceptions

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 4)) {
        throw_ex(cpu, virt, read_address_misaligned_exception, noisy);
    }

    *value = physmem_read32(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

/**
 * @brief Reads 16 bits from virtual memory
 *
 * @param cpu The cpu which makes the read
 * @param virt The virtual address of the read target
 * @param value The pointer where will the read value be stored on success
 * @param fetch If the read is instruction fetch or data read
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_read_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t *value, bool fetch, bool noisy)
{
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);

    if (try_read_memory_mapped_regs_16(cpu, virt, value)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, fetch, noisy);

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 2)) {
        throw_ex(cpu, virt, read_address_misaligned_exception, noisy);
    }

    *value = physmem_read16(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

/**
 * @brief Reads 8 bits from virtual memory
 *
 * @param cpu The cpu which makes the read
 * @param virt The virtual address of the read target
 * @param value The pointer where will the read value be stored on success
 * @param fetch If the read is instruction fetch or data read
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_read_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t *value, bool noisy)
{
    ASSERT(cpu != NULL);
    ASSERT(value != NULL);

    if (try_read_memory_mapped_regs_8(cpu, virt, value)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, false, false, noisy);

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    *value = physmem_read8(cpu->csr.mhartid, phys, true);
    return rv_exc_none;
}

/**
 * @brief Writes 8 bits to the specified virtual address
 *
 * @param cpu The cpu which makes the write
 * @param virt The virtual address
 * @param value The value to be written
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_write_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t value, bool noisy)
{
    ASSERT(cpu != NULL);

    if (try_write_memory_mapped_regs(cpu, virt, value, 8)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, false, noisy);

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (physmem_write8(cpu->csr.mhartid, phys, value, true)) {
        return rv_exc_none;
    }

    // writing to invalid memory
    // throw_ex(cpu, virt, rv_exc_store_amo_access_fault, noisy);
    return rv_exc_none;
}

/**
 * @brief Writes 16 bits to the specified virtual address
 *
 * @param cpu The cpu which makes the write
 * @param virt The virtual address
 * @param value The value to be written
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_write_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t value, bool noisy)
{
    ASSERT(cpu != NULL);

    if (try_write_memory_mapped_regs(cpu, virt, value, 16)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, false, noisy);

    // address translation exceptions have priority to alignment exceptions
    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 2)) {
        throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned, noisy);
    }

    if (physmem_write16(cpu->csr.mhartid, phys, value, true)) {
        return rv_exc_none;
    }

    // writing to invalid memory
    // throw_ex(cpu, virt, rv_exc_store_amo_access_fault, noisy);
    return rv_exc_none;
}

/**
 * @brief Writes 32 bits to the specified virtual address
 *
 * @param cpu The cpu which makes the write
 * @param virt The virtual address
 * @param value The value to be written
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_write_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t value, bool noisy)
{
    ASSERT(cpu != NULL);

    if (try_write_memory_mapped_regs(cpu, virt, value, 32)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, false, noisy);

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 4)) {
        throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned, noisy);
    }

    if (physmem_write32(cpu->csr.mhartid, phys, value, true)) {
        return rv_exc_none;
    }

    // writing to invalid memory
    // throw_ex(cpu, virt, rv_exc_store_amo_access_fault, noisy);
    return rv_exc_none;
}

/**
 * @brief Writes 64 bits to the specified virtual address
 *
 * @param cpu The cpu which makes the write
 * @param virt The virtual address
 * @param value The value to be written
 * @param noisy Shall this operation change the global and cpu state
 * @return rv_exc_t The exception code
 */
static rv_exc_t rv_write_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t value, bool noisy)
{
    ASSERT(cpu != NULL);

    if (try_write_memory_mapped_regs(cpu, virt, value, 64)) {
        return rv_exc_none;
    }

    ptr36_t phys;
    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, false, noisy);

    if (ex != rv_exc_none) {
        throw_ex(cpu, virt, ex, noisy);
    }

    if (!IS_ALIGNED(virt, 8)) {
        throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned, noisy);
    }

    if (physmem_write64(cpu->csr.mhartid, phys, value, true)) {
        return rv_exc_none;
    }

    // writing to invalid memory
    // throw_ex(cpu, virt, rv_exc_store_amo_access_fault, noisy);
    return rv_exc_none;
}

static bool rv_sc_access(rv_cpu_t *cpu, ptr36_t phys, int size)
{
    ASSERT(cpu != NULL);

    ptr36_t res_addr = ((rv_cpu_t *) cpu)->reserved_addr;

    // The size of the reservation is 4B, we check whether the write overlaps
    bool hit = AREAS_OVERLAP(res_addr, 4, phys, size);

    if (hit) {
        ((rv_cpu_t *) cpu)->reserved_valid = false;
    }
    return hit;
}

#undef throw_ex
