/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

#define MAX_CPU  31

/** Argument length */
typedef enum {
	BITS_8 = 1,
	BITS_16 = 2,
	BITS_32 = 4
} wsize_t;

/** Exception types */
typedef enum {
	excInt   = 0,
	excMod   = 1,
	excTLBL  = 2,
	excTLBS  = 3,
	excAdEL  = 4,
	excAdES  = 5,
	excIBE   = 6,
	excDBE   = 7,
	excSys   = 8,
	excBp    = 9,
	excRI    = 10,
	excCpU   = 11,
	excOv    = 12,
	excTr    = 13,
	excVCEI  = 14,
	excFPE   = 15,
	excWATCH = 23,
	excVCED  = 31,
	
	/* Special exception types */
	excTLBR  = 64,
	excTLBLR = 65,
	excTLBSR = 66,
	
	/* For internal usage */
	excNone = 100,
	excAddrError,
	excTLB,
	excReset
} exc_t;

/** Address and length types */
typedef uint32_t ptr32_t;
typedef uint32_t len32_t;

typedef uint64_t ptr36_t;
typedef uint64_t len36_t;

typedef uint64_t ptr64_t;
typedef uint64_t len64_t;

#endif
