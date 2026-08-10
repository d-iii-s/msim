// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _TYPES_H
#define _TYPES_H

/*
 * All-in-one header for common types.
 *
 * Note that the type definitions rely heavily on our
 * knowledge of the compiler configuration.
 *
 * We include static asserts to catch any violations
 * of our assumptions.
 */

#define NULL ((void*)0)

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t native_t;
typedef uint32_t unative_t;
typedef uint32_t uintptr_t;
typedef uint32_t off_t;
typedef uint32_t size_t;
typedef int32_t ssize_t;

#define bool _Bool
#define false 0
#define true 1

#define _CHECK_TYPE_SIZE(type, expected_size) \
    _Static_assert(sizeof(type) == expected_size, #type " should be " #expected_size "B")

_CHECK_TYPE_SIZE(bool, 1);
_CHECK_TYPE_SIZE(int8_t, 1);
_CHECK_TYPE_SIZE(uint8_t, 1);
_CHECK_TYPE_SIZE(int16_t, 2);
_CHECK_TYPE_SIZE(uint16_t, 2);
_CHECK_TYPE_SIZE(int32_t, 4);
_CHECK_TYPE_SIZE(uint32_t, 4);
_CHECK_TYPE_SIZE(int64_t, 8);
_CHECK_TYPE_SIZE(uint64_t, 8);

#endif
