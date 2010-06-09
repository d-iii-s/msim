/*
 * Copyright (c) 2001-2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Some useful types
 *
 */

#ifndef MTYPES_H_
#define MTYPES_H_

#include <stdint.h>

/**< Errors */
#define FE_OPEN_SETUP 1
#define FE_READ_SETUP 2

/**< Argument length */
#define INT8  1
#define INT16 2
#define INT32 4

/**< Exception types */
enum exc { 
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
	excVCEI	 = 14,
	excFPE   = 15,
	excWATCH = 23,
	excVCED	 = 31,

	/* Special exception types */
	excTLBR  = 64,
	excTLBLR = 65,
	excTLBSR = 66,
	
	/* For us */
	excNone	= 100,
	excAddrError,
	excTLB,
	excReset
};

/**< Addressing and length types */
typedef uint32_t ptr_t;
typedef uint32_t len_t;

#endif /* MTYPES_H_ */
