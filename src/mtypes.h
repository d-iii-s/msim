/*
 * mtypes.h
 * some useful types
 * Copyright (c) 2001, 2002, 2003 Viliam Holub
 */


#ifndef _MTYPES_H_
#define _MTYPES_H_

#include <stdint.h>
#include <stdbool.h>

/* errors */
#define FE_OPEN_SETUP	1
#define FE_READ_SETUP	2

#define xfree( x)	{ free( x); x = 0; }

/* argument length */
#define	INT8	1
#define INT16	2
#define	INT32	4

/* exception types */
enum exc
{ 
	excInt	 = 0,
	excMod	 = 1,
	excTLBL	 = 2,
	excTLBS	 = 3,
	excAdEL	 = 4,
	excAdES	 = 5,
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

	/* special exception types */
	excTLBR		= 64,
	excTLBLR	= 65,
	excTLBSR	= 66,
	
	/* for us */
	excNone	= 100,
	excAddrError,
	excTLB,
	excReset
};


#endif /* _MTYPES_H_ */
