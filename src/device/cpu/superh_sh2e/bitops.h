/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Bit operations.
 *
 */

#ifndef BITOPS_H_
#define BITOPS_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif


/****************************************************************************
 * Sign/zero extensions (variable).
 ****************************************************************************/

#define EXTEND_TYPE(dtype, val, bits) \
    (((dtype) ((val) << (8 * sizeof(dtype) - (bits)))) >> (8 * sizeof(dtype) - (bits)))


// Generic variants that allow specialization on the bit count.

static ALWAYS_INLINE uint_fast64_t
__sign_extend(uint_fast64_t const val, unsigned const bits) {
    if (bits <= 8) {
        return EXTEND_TYPE(int8_t, val, bits);
    } else if (bits <= 16) {
        return EXTEND_TYPE(int16_t, val, bits);
    } else if (bits <= 32) {
        return EXTEND_TYPE(int32_t, val, bits);
    } else {
        return EXTEND_TYPE(int_fast64_t, val, bits);
    }
}


static ALWAYS_INLINE uint_fast64_t
__zero_extend(uint_fast64_t const val, unsigned const bits) {
    if (bits <= 8) {
        return EXTEND_TYPE(uint8_t, val, bits);
    } else if (bits <= 16) {
        return EXTEND_TYPE(uint16_t, val, bits);
    } else if (bits <= 32) {
        return EXTEND_TYPE(uint32_t, val, bits);
    } else {
        return EXTEND_TYPE(uint_fast64_t, val, bits);
    }
}


// Generic variants not intended for specialization.

static ALWAYS_INLINE uint_fast64_t
sign_extend(uint_fast64_t const val, unsigned const bits) {
    return EXTEND_TYPE(int_fast64_t, val, bits);
}

static inline uint_fast64_t
zero_extend(uint_fast64_t const val, unsigned const bits) {
    return EXTEND_TYPE(uint_fast64_t, val, bits);
}


/****************************************************************************
 * Sign/zero extensions (fixed).
 ****************************************************************************/

static ALWAYS_INLINE uint16_t
sign_extend_8_16(uint_fast32_t const val) {
    return (int8_t) val;
}

static ALWAYS_INLINE uint32_t
sign_extend_8_32(uint_fast32_t const val) {
    return (int8_t) val;
}

static ALWAYS_INLINE uint32_t
sign_extend_12_32(uint_fast32_t const val) {
    return EXTEND_TYPE(int16_t, val, 12);
}

static ALWAYS_INLINE uint32_t
sign_extend_16_32(uint_fast32_t const val) {
    return (int16_t) val;
}

static ALWAYS_INLINE uint32_t
sign_extend_32_32(uint_fast32_t const val) {
    return (int32_t) val;
}

//

static ALWAYS_INLINE uint32_t
zero_extend_4_32(uint_fast32_t const val) {
    return EXTEND_TYPE(uint8_t, val, 4);
}

static ALWAYS_INLINE uint32_t
zero_extend_8_32(uint_fast32_t const val) {
    return (uint8_t) val;
}

static ALWAYS_INLINE uint32_t
zero_extend_16_32(uint_fast32_t const val) {
    return (uint16_t) val;
}


/****************************************************************************
 * Bit values.
 ****************************************************************************/

static ALWAYS_INLINE uint32_t
bit_value_msb(uint32_t const value) {
    return value >> (8 * sizeof(uint32_t) - 1);
}


static ALWAYS_INLINE uint32_t
bit_value_lsb(uint32_t const value) {
    return value & 1;
}


/****************************************************************************
 * Bit rotations.
 ****************************************************************************/

static ALWAYS_INLINE uint32_t
rotate_left(uint32_t const value, unsigned const amount) {
    return (value << amount) | (value >> ((8 * sizeof(value)) - amount));
}

static ALWAYS_INLINE uint32_t
rotate_right(uint32_t const value, unsigned const amount) {
    return (value << ((8 * sizeof(value)) - amount)) | (value >> amount);
}


/****************************************************************************
 * Miscellaneous
 ****************************************************************************/

/**
 * Returns `true` if an integer value `n` is a power of
 * two, `false` otherwise. Zero is not considered a power of two.
 *
 * A power of two has only one bit set in its binary representation.
 */
#define IS_POWER_OF_TWO(n) (((n) > 0) && __builtin_popcount(n) == 1)

/**
 * Determines which power of two represents the given value `n`.
 * Returns -1 if the value is not a power of two.
 */
#define POWER_OF_TWO(n) (IS_POWER_OF_TWO(n) ? __builtin_ctz(n) : -1)

#endif // BITOPS_H_
