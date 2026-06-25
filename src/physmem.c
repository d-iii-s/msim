/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Physical memory space implementation
 *
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "debug/breakpoint.h"
#include "device/cpu/general_cpu.h"
#include "device/device.h"
#include "endian.h"
#include "list.h"
#include "physmem.h"
#include "utils.h"

/** Physical memory management
 *
 */

#define FTL2_WIDTH 12
#define FTL1_WIDTH 12

#define FTL2_COUNT (1 << FTL2_WIDTH)
#define FTL1_COUNT (1 << FTL1_WIDTH)

#define FTL2_SHIFT (FRAME_WIDTH)
#define FTL1_SHIFT (FRAME_WIDTH + FTL2_WIDTH)

#define FTL2_MASK (FTL2_COUNT - 1)
#define FTL1_MASK (FTL1_COUNT - 1)

typedef frame_t *ftl1_t[FTL2_COUNT];
typedef ftl1_t *ftl0_t[FTL1_COUNT];

static ftl0_t ftl0;

void physmem_wire(physmem_area_t *area)
{
    ASSERT(area != NULL);
    ASSERT(area->type != MEMT_NONE);
    ASSERT(area->count > 0);
    ASSERT(area->data != NULL);
    // ASSERT(area->trans != NULL);

    pfn_t pfn;
    for (pfn = 0; pfn < area->count; pfn++) {
        ptr36_t addr = FRAME2ADDR(area->start + pfn);

        /* 1st level frame table */
        ftl1_t *ftl1 = ftl0[(addr >> FTL1_SHIFT) & FTL1_MASK];
        if (ftl1 == NULL) {
            /* Allocate new 2nd level frame table */
            ftl1 = safe_malloc_t(ftl1_t);
            memset(ftl1, 0, sizeof(ftl1_t));
            ftl0[(addr >> FTL1_SHIFT) & FTL1_MASK] = ftl1;
        }

        /* 2nd level frame table */
        frame_t *frame = (*ftl1)[(addr >> FTL2_SHIFT) & FTL2_MASK];
        if (frame == NULL) {
            /* Allocate new frame descriptor */
            frame = safe_malloc_t(frame_t);
            memset(frame, 0, sizeof(frame_t));
            (*ftl1)[(addr >> FTL2_SHIFT) & FTL2_MASK] = frame;
        }

        /* Frame descriptor */
        frame->area = area;
        frame->data = area->data + FRAMES2SIZE(pfn);
        // frame->trans = area->trans + SIZE2INSTRS(FRAMES2SIZE(pfn));
        frame->valid = false;
    }
}

void physmem_unwire(physmem_area_t *area)
{
    ASSERT(area != NULL);
    ASSERT(area->type != MEMT_NONE);
    ASSERT(area->count > 0);
    ASSERT(area->data != NULL);

    uint32_t pfn;
    for (pfn = 0; pfn < area->count; pfn++) {
        ptr36_t addr = FRAME2ADDR(area->start + pfn);

        /* 1st level frame table */
        ftl1_t *ftl1 = ftl0[(addr >> FTL1_SHIFT) & FTL1_MASK];
        ASSERT(ftl1 != NULL);

        /* 2nd level frame table */
        frame_t **frame_ref = &((*ftl1)[(addr >> FTL2_SHIFT) & FTL2_MASK]);
        ASSERT(*frame_ref != NULL);

        /* Remove frame */
        safe_free(*frame_ref);

        /* Deallocate ftl1 if it contains only NULL entries*/
        bool ftl1_empty = true;

        for (size_t i = 0; i < sizeof(*ftl1) / sizeof(frame_t *); ++i) {
            if ((*ftl1)[i] != NULL) {
                ftl1_empty = false;
                break;
            }
        }

        if (ftl1_empty) {
            safe_free(ftl1);
        }
    }
}

frame_t *physmem_find_frame(ptr36_t addr)
{
    ftl1_t *ftl1 = ftl0[(addr >> FTL1_SHIFT) & FTL1_MASK];
    if (ftl1 != NULL) {
        frame_t *frame = (*ftl1)[(addr >> FTL2_SHIFT) & FTL2_MASK];
        if (frame != NULL) {
            return frame;
        }
    }

    return NULL;
}

/** Find an activated memory breakpoint
 *
 * Find an activated memory breakpoint which would be hit for specified
 * memory address and access conditions.
 *
 * @param addr         Address, where the breakpoint can be hit.
 * @param size         Size of the access operation.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 * @return Found breakpoint structure or NULL if there is not any.
 *
 */
static void physmem_breakpoint_find(ptr36_t addr, len36_t size,
        access_t access_type)
{
    physmem_breakpoint_t *breakpoint;

    for_each(physmem_breakpoints, breakpoint, physmem_breakpoint_t)
    {
        if (breakpoint->addr + breakpoint->size < addr) {
            continue;
        }

        if (breakpoint->addr > addr + breakpoint->size) {
            continue;
        }

        if ((access_type & breakpoint->access_flags) != 0) {
            physmem_breakpoint_hit(breakpoint, access_type);
            break;
        }
    }
}

static uint8_t devmem_read8(unsigned int procno, ptr36_t addr)
{
    uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->read8) {
            dev->type->read8(procno, dev, addr, (uint8_t *) &val);
        } else if (dev->type->read32) {
            dev->type->read32(procno, dev, addr, &val);
        }
    }

    return val;
}

static uint16_t devmem_read16(unsigned int procno, ptr36_t addr)
{
    uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->read16) {
            dev->type->read16(procno, dev, addr, (uint16_t *) &val);
        } else if (dev->type->read32) {
            dev->type->read32(procno, dev, addr, &val);
        }
    }

    return val;
}

static uint32_t devmem_read32(unsigned int procno, ptr36_t addr)
{
    uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->read32) {
            dev->type->read32(procno, dev, addr, &val);
        }
    }

    return val;
}

static uint64_t devmem_read64(unsigned int procno, ptr36_t addr)
{
    uint64_t val = (uint64_t) DEFAULT_MEMORY_VALUE;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->read64) {
            dev->type->read64(procno, dev, addr, &val);
        }
    }

    return val;
}

/** Physical memory read (8 bits)
 *
 * Read 8 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param procno    Id of processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint8_t physmem_read8(unsigned int procno, ptr36_t addr, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /*
     * No memory frame found, try to read the value
     * from appropriate device or return the default value.
     */
    if (frame == NULL) {
        return devmem_read8(procno, addr);
    }

    /* Check for memory read breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 1, ACCESS_READ);
    }

    ASSERT(frame->data);
    uint8_t *data = frame->data + (addr & FRAME_MASK);

    return convert_uint8_t_endian(*data);
}

/** Physical memory read (16 bits)
 *
 * Read 16 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param procno    Id of processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint16_t physmem_read16(unsigned int procno, ptr36_t addr, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /*
     * No memory frame found, try to read the value
     * from appropriate device or return the default value.
     */
    if (frame == NULL) {
        return devmem_read16(procno, addr);
    }

    /* Check for memory read breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 2, ACCESS_READ);
    }

    ASSERT(frame->data);
    uint16_t *data = (uint16_t *) (frame->data + (addr & FRAME_MASK));

    // TODO Fix conversion to take into account simulated CPU.
    return convert_uint16_t_endian(*data);
}

/** Physical memory read (32 bits)
 *
 * Read 32 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param procno    Id of processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint32_t physmem_read32(unsigned int procno, ptr36_t addr, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /*
     * No memory frame found, try to read the value
     * from appropriate device or return the default value.
     */
    if (frame == NULL) {
        return devmem_read32(procno, addr);
    }

    /* Check for memory read breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 4, ACCESS_READ);
    }

    ASSERT(frame->data);
    uint32_t *data = (uint32_t *) (frame->data + (addr & FRAME_MASK));

    // TODO Fix conversion to take into account simulated CPU.
    return convert_uint32_t_endian(*data);
}

/** Physical memory read (64 bits)
 *
 * Read 64 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param procno    Id of processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint64_t physmem_read64(unsigned int procno, ptr36_t addr, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /*
     * No memory frame found, try to read the value
     * from appropriate device or return the default value.
     */
    if (frame == NULL) {
        return devmem_read64(procno, addr);
    }

    /* Check for memory read breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 8, ACCESS_READ);
    }

    ASSERT(frame->data);
    uint64_t *data = (uint64_t *) (frame->data + (addr & FRAME_MASK));

    return convert_uint64_t_endian(*data);
}

static bool devmem_write8(unsigned int procno, ptr36_t addr, uint8_t val)
{
    bool written = false;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->write8) {
            dev->type->write8(procno, dev, addr, val);
            written = true;
        } else if (dev->type->write32) {
            /* Some devices may only support 32-bit writes */
            dev->type->write32(procno, dev, addr, (uint32_t) val);
            written = true;
        }
    }

    return written;
}

static bool devmem_write16(unsigned int procno, ptr36_t addr, uint16_t val)
{
    bool written = false;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->write16) {
            dev->type->write16(procno, dev, addr, val);
            written = true;
        } else if (dev->type->write32) {
            /* Some devices may only support 32-bit writes */
            uint32_t value = (uint32_t) val;
            dev->type->write32(procno, dev, addr, value);
            written = true;
        }
    }

    return written;
}

static bool devmem_write32(unsigned int procno, ptr36_t addr, uint32_t val)
{
    bool written = false;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->write32) {
            dev->type->write32(procno, dev, addr, val);
            written = true;
        }
    }

    return written;
}

static bool devmem_write64(unsigned int procno, ptr36_t addr, uint64_t val)
{
    bool written = false;

    /* List for each device */
    device_t *dev = NULL;
    while (dev_next(&dev, DEVICE_FILTER_ALL)) {
        if (dev->type->write64) {
            dev->type->write64(procno, dev, addr, val);
            written = true;
        }
    }

    return written;
}

/** SC-LL tracking
 *
 */

typedef struct {
    item_t item;
    unsigned int procno;
} sc_item_t;

static list_t sc_list = LIST_INITIALIZER;

/** Register current processor in LL-SC tracking list
 *
 */
void sc_register(unsigned int procno)
{
    /* Ignore if already registered. */
    sc_item_t *sc_item;

    for_each(sc_list, sc_item, sc_item_t)
    {
        if (sc_item->procno == procno) {
            return;
        }
    }

    sc_item = safe_malloc_t(sc_item_t);
    item_init(&sc_item->item);
    sc_item->procno = procno;
    list_append(&sc_list, &sc_item->item);
}

/** Remove current processor from the LL-SC tracking list
 *
 */
void sc_unregister(unsigned int procno)
{
    sc_item_t *sc_item;

    for_each(sc_list, sc_item, sc_item_t)
    {
        if (sc_item->procno == procno) {
            list_remove(&sc_list, &sc_item->item);
            safe_free(sc_item);
            break;
        }
    }
}

/** Load Linked and Store Conditional control
 *
 */
static void sc_control(ptr36_t addr, int size)
{
    sc_item_t *sc_item = (sc_item_t *) sc_list.head;

    while (sc_item != NULL) {

        if (cpu_sc_access(get_cpu(sc_item->procno), addr, size)) {
            sc_item_t *tmp = sc_item;
            sc_item = (sc_item_t *) sc_item->item.next;

            list_remove(&sc_list, &tmp->item);
            safe_free(tmp);
        } else {
            sc_item = (sc_item_t *) sc_item->item.next;
        }
    }
}

/** Physical memory write (8 bits)
 *
 * Write 8 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param procno    Id of processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write8(unsigned int procno, ptr36_t addr, uint8_t val, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /* No frame found, try to write the value to appropriate device */
    if (frame == NULL) {
        return devmem_write8(procno, addr, val);
    }

    ASSERT(frame->area);
    ASSERT(frame->data);

    /* Writting to ROM? */
    if ((!frame->area->writable) && (protected)) {
        return false;
    }

    sc_control(addr, 1);

    /* Check for memory write breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 1, ACCESS_WRITE);
    }

    /* Invalidate binary translation */
    frame->valid = false;

    uint8_t *data = frame->data + (addr & FRAME_MASK);
    *data = convert_uint8_t_endian(val);

    return true;
}

/** Physical memory write (16 bits)
 *
 * Write 16 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param procno    Id of processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write16(unsigned int procno, ptr36_t addr, uint16_t val, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /* No frame found, try to write the value to appropriate device */
    if (frame == NULL) {
        return devmem_write16(procno, addr, val);
    }

    ASSERT(frame->area);
    ASSERT(frame->data);

    /* Writting to ROM? */
    if ((!frame->area->writable) && (protected)) {
        return false;
    }

    sc_control(addr, 2);

    /* Check for memory write breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 2, ACCESS_WRITE);
    }

    /* Invalidate binary translation */
    frame->valid = false;

    uint16_t *data = (uint16_t *) (frame->data + (addr & FRAME_MASK));
    *data = convert_uint16_t_endian(val);

    return true;
}

/** Physical memory write (32 bits)
 *
 * Write 32 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param procno    Id of processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write32(unsigned int procno, ptr36_t addr, uint32_t val, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /* No frame found, try to write the value to appropriate device */
    if (frame == NULL) {
        return devmem_write32(procno, addr, val);
    }

    ASSERT(frame->area);
    ASSERT(frame->data);

    /* Writting to ROM? */
    if ((!frame->area->writable) && (protected)) {
        return false;
    }

    sc_control(addr, 4);

    /* Check for memory write breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 4, ACCESS_WRITE);
    }

    /* Invalidate binary translation */
    frame->valid = false;

    uint32_t *data = (uint32_t *) (frame->data + (addr & FRAME_MASK));
    *data = convert_uint32_t_endian(val);

    return true;
}

/** Physical memory write (64 bits)
 *
 * Write 64 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param procno    Id of processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write64(unsigned int procno, ptr36_t addr, uint64_t val, bool protected)
{
    frame_t *frame = physmem_find_frame(addr);

    /* No frame found, try to write the value to appropriate device */
    if (frame == NULL) {
        return devmem_write64(procno, addr, val);
    }

    ASSERT(frame->area);
    ASSERT(frame->data);

    /* Writting to ROM? */
    if ((!frame->area->writable) && (protected)) {
        return false;
    }

    sc_control(addr, 8);

    /* Check for memory write breakpoints */
    if (protected) {
        physmem_breakpoint_find(addr, 8, ACCESS_WRITE);
    }

    /* Invalidate binary translation */
    frame->valid = false;

    uint64_t *data = (uint64_t *) (frame->data + (addr & FRAME_MASK));
    *data = convert_uint64_t_endian(val);

    return true;
}
