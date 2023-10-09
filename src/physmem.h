/*
 * Copyright (c) 2004-2007 Viliam Holub
 * Copyright (c) 2008-2011 Martin Decky
 * Copyright (c) 2022      Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Physical memory
 *
 */

#ifndef PHYSMEM_H_
#define PHYSMEM_H_

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "main.h"
#include "list.h"
#include "utils.h"

#define FRAME_WIDTH   12
#define FRAME_SIZE    (1 << FRAME_WIDTH)
#define FRAME_MASK    (FRAME_SIZE - 1)


#define FRAMES2SIZE(frames) \
    (((len36_t) (frames)) << FRAME_WIDTH)

#define SIZE2FRAMES(size) \
    ((size) >> FRAME_WIDTH)

#define FRAME2ADDR(frame) \
    (((ptr36_t) (frame)) << FRAME_WIDTH)

#define ADDR2FRAME(addr) \
    ((addr) >> FRAME_WIDTH)

#define DEFAULT_MEMORY_VALUE  UINT64_C(0xffffffffffffffff)

typedef enum {
    MEMT_NONE = 0,  /**< Uninitialized */
    MEMT_MEM  = 1,  /**< Generic */
    MEMT_FMAP = 2   /**< File mapped */
} physmem_type_t;

typedef struct {
    /* Memory area type */
    physmem_type_t type;
    bool writable;

    /* Starting physical frame */
    pfn_t start;

    /* Number of physical frames */
    pfn_t count;

    /* Memory content */
    uint8_t *data;
} physmem_area_t;

typedef struct frame {
    /* Physical memory area containing the frame */
    physmem_area_t *area;

    /* Frame data (with displacement) */
    uint8_t *data;

    /* Binary translation valid flag */
    bool valid;
} frame_t;

/** Physical memory management */
extern void physmem_wire(physmem_area_t *area);
extern void physmem_unwire(physmem_area_t *area);

extern frame_t* physmem_find_frame(ptr36_t addr);

/** Physical memory access */
extern uint8_t physmem_read8(unsigned int cpu, ptr36_t addr, bool protected);
extern uint16_t physmem_read16(unsigned int cpu, ptr36_t addr, bool protected);
extern uint32_t physmem_read32(unsigned int cpu, ptr36_t addr, bool protected);
extern uint64_t physmem_read64(unsigned int cpu, ptr36_t addr, bool protected);

extern bool physmem_write8(unsigned int cpu, ptr36_t addr, uint8_t val,
    bool protected);
extern bool physmem_write16(unsigned int cpu, ptr36_t addr, uint16_t val,
    bool protected);
extern bool physmem_write32(unsigned int cpu, ptr36_t addr, uint32_t val,
    bool protected);
extern bool physmem_write64(unsigned int cpu, ptr36_t addr, uint64_t val,
    bool protected);

/** Store-conditional control */
extern void sc_register(unsigned int procno);
extern void sc_unregister(unsigned int procno);

#endif /* PHYSMEM_H_ */