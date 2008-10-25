/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MMAP_H_
#define MMAP_H_

#ifdef __WIN32__

/**
 *
 * Dummy definitions for compatibility
 * with sys/mman.h.
 *
 */

#define mmap(addr, length, prot, flags, fd, offset)  NULL
#define munmap(addr, length)  0

#define MAP_FAILED  ((void *) -1)

#else

#include <sys/mman.h>

#endif /* __WIN32__ */

#endif /* MMAP_H_ */
