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

#define MAP_FAILED  ((void *) -1)

extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int munmap(void *addr, size_t length);

#else

#include <sys/mman.h>

#endif /* __WIN32__ */

#endif /* MMAP_H_ */
