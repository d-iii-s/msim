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

#include <stdbool.h>
#include <stdint.h>

#include "exception.h"

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#if XLEN == 64
typedef uint64_t uxlen_t;
typedef uint64_t virt_t;
typedef int64_t xlen_t;
#elif XLEN == 32
typedef uint32_t uxlen_t;
typedef uint32_t virt_t;
typedef int32_t xlen_t;
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

#if XLEN == 64
#define XLEN_MIN INT64_MIN
#define XLEN_UMAX UINT64_MAX
#else
#define XLEN_MIN INT32_MIN
#define XLEN_UMAX UINT32_MAX
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

/* Multiplication utilities */

static inline uxlen_t uxlen_mulhuu(uxlen_t a, uxlen_t b)
{
#if XLEN == 64

#if defined(__SIZEOF_INT128__)
    unsigned __int128 x = a;
    unsigned __int128 y = b;
    unsigned __int128 result = x * y;
    return result >> 64;
#else
    uint64_t a_lo = (uint32_t) (a & 0xffffffff);
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = (uint32_t) (b & 0xffffffff);
    uint64_t b_hi = b >> 32;

    uint64_t res_lo = a_lo * b_lo;
    uint64_t mid = a_hi * b_lo + (res_lo >> 32);
    uint64_t mid_lo = (uint32_t) (mid & 0xffffffff);
    uint64_t mid_hi = mid >> 32;
    uint64_t mid_lo2 = mid_lo + a_lo * b_hi;

    uint64_t res_hi = a_hi * b_hi + mid_hi + (mid_lo2 >> 32);
    return res_hi;
#endif

#elif XLEN == 32

    uint64_t result = ((uint64_t) a) * ((uint64_t) b);
    return result >> 32;

#else
#error "XLEN is not set to 32 or 64 bits"
#endif
}

static inline uxlen_t uxlen_mulhsu(xlen_t a, uxlen_t b)
{
#if XLEN == 64

#if defined(__SIZEOF_INT128__)
    __int128 x = a;
    unsigned __int128 y = b;
    unsigned __int128 result = x * y;
    return result >> 64;
#else
    uint64_t ua = (uint64_t) a;
    uint64_t hi = uxlen_mulhuu(ua, b);
    if (a < 0) {
        hi -= b;
    }
    return hi;
#endif

#elif XLEN == 32

    int64_t result = ((int64_t) a) * ((uint64_t) b);
    return result >> 32;

#else
#error "XLEN is not set to 32 or 64 bits"
#endif
}

static inline uxlen_t uxlen_mulhss(xlen_t a, xlen_t b)
{
#if XLEN == 64

#if defined(__SIZEOF_INT128__)
    __int128 x = a;
    __int128 y = b;
    __int128 result = x * y;
    return result >> 64;
#else
    uint64_t ua = (uint64_t) a;
    uint64_t ub = (uint64_t) b;
    uint64_t hi = uxlen_mulhuu(ua, ub);
    if (a < 0) {
        hi -= ub;
    }
    if (b < 0) {
        hi -= ua;
    }
    return hi;
#endif

#elif XLEN == 32

    int64_t result = ((int64_t) a) * ((int64_t) b);
    return result >> 32;

#else
#error "XLEN is not set to 32 or 64 bits"
#endif
}

#endif // RISCV_RV_COMMON_TYPES_H_
