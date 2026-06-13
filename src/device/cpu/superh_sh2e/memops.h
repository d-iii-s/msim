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

#ifndef SUPERH_SH2E_MEMOPS_H_
#define SUPERH_SH2E_MEMOPS_H_

#include <stdint.h>

#include "../../../arch/endianness.h"
#include "../../../main.h"
#include "../../../physmem.h"
#include "bitops.h"

static inline uint8_t
sh2e_physmem_read8(
        unsigned int const cpu_id, uint32_t const addr, bool const protected)
{
    return physmem_read8(cpu_id, addr, protected);
}

static inline uint16_t
sh2e_physmem_read16(
        unsigned int const cpu_id, uint32_t const addr, bool const protected)
{
    return be16toh(physmem_read16(cpu_id, addr, protected));
}

static inline uint32_t
sh2e_physmem_read32(
        unsigned int const cpu_id, uint32_t const addr, bool const protected)
{
    return be32toh(physmem_read32(cpu_id, addr, protected));
}

/**
 * @brief Reads an 8-bit value (byte) from memory, zero-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param output_value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_readz_byte(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        uint32_t *const restrict output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    *output_value = sh2e_physmem_read8(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Reads an 8-bit value (byte) from memory, sign-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param output_value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_reads_byte(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        uint32_t *const restrict output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    *output_value = sign_extend_8_32(sh2e_physmem_read8(cpu->id, addr, true));
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Reads a 16-bit value (word) from memory, zero-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_readz_word(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        uint32_t *const restrict output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // Check alignment (must be even address).
    if (addr & 1) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    *output_value = sh2e_physmem_read16(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Reads a 16-bit value (word) from memory, sign-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_reads_word(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        uint32_t *const restrict output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // Check alignment (must be even address).
    if (addr & 1) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    *output_value = sign_extend_16_32(sh2e_physmem_read16(cpu->id, addr, true));
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Reads a long word (32 bits) from memory.
 *
 * @param cpu The cpu which makes the read.
 * @param addr The (physical) memory address from which to load data.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_read_long(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        uint32_t *const restrict output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // Check alignment (must be multiple of 4)
    if (addr & 3) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    *output_value = sh2e_physmem_read32(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}

/**
 * @brief Reads a float (32 bits) from memory.
 *
 * @param cpu The cpu which makes the read.
 * @param addr The (physical) memory address from which to load data.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_read_float(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr,
        float32_t *const output_value)
{
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // Check alignment (must be multiple of 4)
    if (addr & 3) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    uint32_t *const output_uint32 = (uint32_t *) output_value;
    *output_uint32 = sh2e_physmem_read32(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}

/** @brief Writes a byte value (8 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_byte(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr, uint32_t const value)
{
    ASSERT(cpu != NULL);

    // While testing exceptions for the instructions, we assume that the write will succeed.
    // If the write fails in the real implementation, the problem is that the value is written
    // into ROM or no memory is allocated at the address, which is not a problem of the instruction
    // itself but rather a problem in the configuration of MSIM. All CPU address exceptions will be
    // raised before this if-else block.
    if (machine_unit_testing) {
        return SH2E_EXCEPTION_NONE;
    }

    bool success = physmem_write8(cpu->id, addr, value, true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}

/** @brief Writes a word value (16 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_word(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr, uint32_t const value)
{
    ASSERT(cpu != NULL);

    // Check alignment (must be even address).
    if (addr & 1) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    // While testing exceptions for the instructions, we assume that the write will succeed.
    // If the write fails in the real implementation, the problem is that the value is written
    // into ROM or no memory is allocated at the address, which is not a problem of the instruction
    // itself but rather a problem in the configuration of MSIM. All alignment exceptions will be
    // raised before this if-else block.
    if (machine_unit_testing) {
        return SH2E_EXCEPTION_NONE;
    }

    bool success = physmem_write16(cpu->id, addr, htobe16(value), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}

/** @brief Writes a long value (32 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_long(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr, uint32_t const value)
{
    ASSERT(cpu != NULL);

    // Check alignment (must be multiple of 4)
    if (addr & 3) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    // While testing exceptions for the instructions, we assume that the write will succeed.
    // If the write fails in the real implementation, the problem is that the value is written
    // into ROM or no memory is allocated at the address, which is not a problem of the instruction
    // itself but rather a problem in the configuration of MSIM. All alignment exceptions will be
    // raised before this if-else block.
    if (machine_unit_testing) {
        return SH2E_EXCEPTION_NONE;
    }

    bool success = physmem_write32(cpu->id, addr, htobe32(value), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}

/** @brief Writes a float value (32 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_float(
        sh2e_cpu_t const *const restrict cpu, uint32_t const addr, float32_t const value)
{
    ASSERT(cpu != NULL);

    // Check alignment (must be multiple of 4)
    if (addr & 3) {
        return SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
    }

    // While testing exceptions for the instructions, we assume that the write will succeed.
    // If the write fails in the real implementation, the problem is that the value is written
    // into ROM or no memory is allocated at the address, which is not a problem of the instruction
    // itself but rather a problem in the configuration of MSIM. All alignment exceptions will be
    // raised before this if-else block.
    if (machine_unit_testing) {
        return SH2E_EXCEPTION_NONE;
    }

    sh2e_fpu_ul_t const conv = { .fvalue = value };
    bool success = physmem_write32(cpu->id, addr, htobe32(conv.ivalue), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}

#endif // SUPERH_SH2E_MEMOPS_H_
