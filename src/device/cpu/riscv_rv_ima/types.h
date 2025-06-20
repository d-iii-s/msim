/*
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Generic types
 *
 */

#pragma GCC diagnostic ignored "-Woverflow"

#ifndef RISCV_RV_COMMON_TYPES_H_
#define RISCV_RV_COMMON_TYPES_H_

#include <stdint.h>

#include "exception.h"

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#if XLEN == 64
typedef uint64_t uxlen_t;
typedef uint64_t virt_t;
typedef int64_t xlen_t;
typedef __int128 bigxlen_t;
typedef unsigned __int128 ubigxlen_t;
#elif XLEN == 32
typedef uint32_t uxlen_t;
typedef uint32_t virt_t;
typedef int32_t xlen_t;
typedef int64_t bigxlen_t;
typedef uint64_t ubigxlen_t;
#else
#error "XLEN is not set to 32 or 64 bits"
#endif

#if XLEN == 64
struct rv64_cpu;
typedef struct rv64_cpu rv_cpu_t;
#else
struct rv32_cpu;
typedef struct rv32_cpu rv_cpu_t;
#endif

static ALWAYS_INLINE xlen_t sign_extend_8_to_xlen(uint8_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 8 bit value to 32 bits (xlen_t = int32_t)
        return (int32_t) ((int16_t) ((int8_t) value));
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (int64_t) ((int32_t) ((int16_t) ((int8_t) value)));
    } else {
        return (int64_t) ((int32_t) ((int16_t) ((int8_t) value)));
    }
}

static ALWAYS_INLINE xlen_t sign_extend_16_to_xlen(uint16_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 16 bit value to 32 bits (xlen_t = int32_t)
        return (int32_t) ((int16_t) value);
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (int64_t) ((int32_t) ((int16_t) value));
    } else {
        return (int64_t) ((int32_t) ((int16_t) value));
    }
}

static ALWAYS_INLINE xlen_t sign_extend_32_to_xlen(uint32_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 32 bit value to 32 bits (xlen_t = int32_t)
        return (int32_t) value;
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (int64_t) ((int32_t) value);
    } else {
        return (int64_t) ((int32_t) value);
    }
}

static ALWAYS_INLINE xlen_t sign_extend_64_to_xlen(uint64_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 64 bit value to 32 bits (xlen_t = int32_t) = TRUNCATION
        return (int32_t) value;
    } else if (xlen == 64) {
        // Sign extend 64 bit value to 64 bits (xlen_t = int64_t) = just a cast
        return (int64_t) value;
    } else {
        return (int64_t) value;
    }
}

/** ZERO extension */

static ALWAYS_INLINE uxlen_t zero_extend_8_to_xlen(uint8_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 8 bit value to 32 bits (xlen_t = int32_t)
        return (uint32_t) ((uint16_t) ((uint8_t) value));
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (uint64_t) ((uint32_t) ((uint16_t) ((uint8_t) value)));
    } else {
        return (uint64_t) ((uint32_t) ((uint16_t) ((uint8_t) value)));
    }
}

static ALWAYS_INLINE uxlen_t zero_extend_16_to_xlen(uint16_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 16 bit value to 32 bits (xlen_t = int32_t)
        return (uint32_t) ((uint16_t) value);
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (uint64_t) ((uint32_t) ((uint16_t) value));
    } else {
        return (uint64_t) ((uint32_t) ((uint16_t) value));
    }
}

static ALWAYS_INLINE xlen_t zero_extend_32_to_xlen(uint32_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 32 bit value to 32 bits (xlen_t = int32_t)
        return (uint32_t) value;
    } else if (xlen == 64) {
        // Sign extend 8 bit value to 64 bits (xlen_t = int64_t)
        return (uint64_t) ((uint32_t) value);
    } else {
        return (uint64_t) ((uint32_t) value);
    }
}

static ALWAYS_INLINE uxlen_t zero_extend_64_to_xlen(uint64_t const value, int const xlen)
{
    if (xlen == 32) {
        // Sign extend 64 bit value to 32 bits (xlen_t = int32_t) = TRUNCATION
        return (uint32_t) value;
    } else if (xlen == 64) {
        // Sign extend 64 bit value to 64 bits (xlen_t = int64_t) = just a cast
        return (uint64_t) value;
    } else {
        return (uint64_t) value;
    }
}

#if XLEN == 64
#define XLEN_C UINT64_C
#else
#define XLEN_C UINT32_C
#endif

/** Address alignment constants */

#define alignment_by_XLEN (XLEN == 32 ? 4 : (XLEN == 64 ? 8 : 8))

/** Read and write operation aliases by XLEN */

typedef rv_exc_t (*rv_read_xlen_t)(rv_cpu_t *, virt_t, uxlen_t *, bool, bool);

/** Instruction constants */

static ALWAYS_INLINE int shift_instr_mask(int const xlen)
{
    if (xlen == 32) {
        return 0x1F;
    } else if (xlen == 64) {
        return 0x3F;
    } else {
        return 0x3F;
    }
}

static ALWAYS_INLINE xlen_t xlen_min(int const xlen)
{
    if (xlen == 32) {
        return INT32_MIN;
    } else if (xlen == 64) {
        return INT64_MIN;
    } else {
        return INT64_MIN;
    }
}

static ALWAYS_INLINE xlen_t uxlen_max(int const xlen)
{
    if (xlen == 32) {
        return UINT32_MAX;
    } else if (xlen == 64) {
        return UINT64_MAX;
    } else {
        return UINT64_MAX;
    }
}

#endif // RISCV_RV_COMMON_TYPES_H_
