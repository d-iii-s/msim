/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MMAP_H_
#define MMAP_H_

#ifdef MINGW

#else

	#include <sys/mman.h>

#endif /* MINGW */

#endif /* NETWORK_H_ */
