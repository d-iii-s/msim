/*
 * Copyright (c) 2010 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#ifndef NDEBUG

#include "fault.h"

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            die(ERR_INTERN, "Assertion failed (%s) in file '%s' " \
                    "line %u", #expr, __FILE__, __LINE__); \
        } \
    } while (0)

#else /* !NDEBUG */

#define ASSERT(expr)

#endif /* !NDEBUG */

#endif
