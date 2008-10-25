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

#define MAP_FAILED   ((void *) -1)

#define PROT_READ    0x01
#define PROT_WRITE   0x02
#define PROT_EXEC    0x04
#define PROT_NONE    0x00

#define MAP_SHARED   0x01
#define MAP_PRIVATE  0x02

extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int munmap(void *addr, size_t length);

#else

#include <sys/mman.h>

#endif /* __WIN32__ */

#endif /* MMAP_H_ */
