/**
 * Copyright (c) 2010 Tomas Martinec
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 * Implementation of breakpoints.
 *
 * The simulator supports memory breakpoints for stopping after r/w access
 * to specified address in the memory and code breakpoints for stopping
 * after the specified processor executes instruction on the specified
 * address. (stopping after is probably a wrong behavior, because the gdb
 * expects stopping before the execution of instruction) The memory
 *
 * The memory and code breakpoints have independent implementation. The memory
 * breakpoints implementation resides completely in this module. Addition or
 * removal of code breakpoints is not implemented here, because the code
 * breakpoints list is associated with each processor structure. The code
 * and memory breakpoints have some similar things, which could be united.
 *
 * The memory breakpoints can be currently set only from the simulator.
 * The user of the simulator is notified after the breakpoint hit and
 * then the simulator waits for next commands. They work with physical address.
 *
 * The code breakpoints can be set from a debugger or from the simulator
 * and the handler for breakpoint hit differs according to it. Debugger
 * breakpoint hit is handled by notifying the debugger and waiting for
 * the next command from the debugger. Simulator breakpoint hit is handled
 * by showing message to the user of the simulator and waiting for next command
 * from him. They work with virtual address.
 */

#include <inttypes.h>

#include "../assert.h"
#include "../device/cpu/general_cpu.h"
#include "../device/cpu/mips_r4000/cpu.h"
#include "../device/cpu/riscv_rv32ima/cpu.h"
#include "../device/device.h"
#include "../device/dr4kcpu.h"
#include "../device/drvcpu.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"
#include "breakpoint.h"
#include "gdb.h"

list_t physmem_breakpoints = LIST_INITIALIZER;

/************************************************************************/
/* Memory breakpoints                                                   */
/************************************************************************/

/** Allocate and initialize a memory breakpoint
 *
 * @param address      Address, where the breakpoint can be hit.
 * @param size         Size of the breakpoint area.
 * @param kind         Specifies if the breakpoint was initiated for the simulator of
 *                     from the debugger.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 * @return Initialized memory breakpoint structure.
 *
 */
static physmem_breakpoint_t *physmem_breakpoint_init(ptr36_t address,
        len36_t size, breakpoint_kind_t kind, access_filter_t access_flags)
{
    physmem_breakpoint_t *breakpoint = safe_malloc_t(physmem_breakpoint_t);

    item_init(&breakpoint->item);
    breakpoint->kind = kind;
    breakpoint->addr = address;
    breakpoint->size = size;
    breakpoint->hits = 0;
    breakpoint->access_flags = access_flags;

    return breakpoint;
}

/** Setup and activate a new memory breakpoint
 *
 * @param address      Address, where the breakpoint can be hit.
 * @param length       Size of the breakpoint area.
 * @param kind         Specifies if the breakpoint was initiated for the simulator of
 *                     from the debugger.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 */
void physmem_breakpoint_add(ptr36_t address, len36_t length,
        breakpoint_kind_t kind, access_filter_t access_flags)
{
    physmem_breakpoint_t *breakpoint = physmem_breakpoint_init(address, length, kind, access_flags);

    list_append(&physmem_breakpoints, &breakpoint->item);
}

/** Deactivate memory breakpoint with specified address
 *
 * @param address Address, where the breakpoint can be hit.
 *
 * @return True, if some breakpoint has been deactivated.
 *
 */
bool physmem_breakpoint_remove(ptr36_t address)
{
    physmem_breakpoint_t *breakpoint = (physmem_breakpoint_t *) physmem_breakpoints.head;

    while (breakpoint != NULL) {
        if (breakpoint->addr == address) {
            list_remove(&physmem_breakpoints, &breakpoint->item);
            safe_free(breakpoint);

            return true;
        }

        breakpoint = (physmem_breakpoint_t *) breakpoint->item.next;
    }

    return false;
}

/** Deactivate all memory breakpoints which matches the given filter.
 *
 * @param filter Filter for selecting breakpoints to be deactivated.
 *
 */
void physmem_breakpoint_remove_filtered(breakpoint_filter_t filter)
{
    physmem_breakpoint_t *breakpoint = (physmem_breakpoint_t *) physmem_breakpoints.head;

    while (breakpoint != NULL) {
        physmem_breakpoint_t *removed = breakpoint;
        breakpoint = (physmem_breakpoint_t *) breakpoint->item.next;

        /* Ignore breakpoints which are not filtered */
        if ((removed->kind & filter) == 0) {
            continue;
        }

        list_remove(&physmem_breakpoints, &removed->item);
        safe_free(removed);
    }
}

/** Print activated memory breakpoints for the user */
void physmem_breakpoint_print_list(void)
{
    printf("[address         ] [mode] [hits              ]\n");

    physmem_breakpoint_t *breakpoint = NULL;
    for_each(physmem_breakpoints, breakpoint, physmem_breakpoint_t)
    {
        bool read = breakpoint->access_flags & ACCESS_READ;
        bool write = breakpoint->access_flags & ACCESS_WRITE;

        printf("%#018" PRIx64 " %c%c     %20" PRIu64 "\n",
                breakpoint->addr,
                read ? 'r' : '-', write ? 'w' : '-',
                breakpoint->hits);
    }
}

/** Fire given memory breakpoint.
 *
 * For simulator breakpoints it is printed appropriate message to console
 * and for debugger breakpoints the debugger is notified.
 *
 * @param breakpoint  Breakpoint to be fired.
 * @param access_type Specifies type of access operation.
 *
 */
void physmem_breakpoint_hit(physmem_breakpoint_t *breakpoint,
        access_t access_type)
{
    ASSERT(breakpoint != NULL);

    switch (breakpoint->kind) {
    case BREAKPOINT_KIND_SIMULATOR:
        if (access_type == ACCESS_READ) {
            alert("Debug: Read from address %#0" PRIx64,
                    breakpoint->addr);
        } else {
            alert("Debug: Written to address %#0" PRIx64,
                    breakpoint->addr);
        }

        breakpoint->hits++;
        machine_interactive = true;
        break;
    case BREAKPOINT_KIND_DEBUGGER:
        gdb_handle_event(GDB_EVENT_BREAKPOINT);
        break;
    default:
        die(ERR_INTERN, "Unexpected physical memory breakpoint kind");
    }
}

/************************************************************************/
/* Code breakpoints                                                     */
/************************************************************************/

/** Allocate and initialize a code breakpoint
 *
 * @param address Address, where the breakpoint can be hit.
 * @param kind    Specifies, how the breakpoint hit will be handled.
 *
 * @return Initialized code breakpoint structure.
 *
 */
breakpoint_t *breakpoint_init(ptr64_t address, breakpoint_kind_t kind)
{
    breakpoint_t *breakpoint = (breakpoint_t *) safe_malloc_t(breakpoint_t);

    item_init(&breakpoint->item);
    breakpoint->pc = address;
    breakpoint->hits = 0;
    breakpoint->kind = kind;

    return breakpoint;
}

/** Fires given breakpoint
 *
 * @param breakpoint Breakpoint structure to be fired
 *
 */
static void breakpoint_hit(breakpoint_t *breakpoint)
{
    breakpoint->hits++;

    switch (breakpoint->kind) {
    case BREAKPOINT_KIND_SIMULATOR:
        alert("Debug: Hit breakpoint at %#0" PRIx64, breakpoint->pc.ptr);
        machine_interactive = true;
        break;
    case BREAKPOINT_KIND_DEBUGGER:
        gdb_handle_event(GDB_EVENT_BREAKPOINT);
        break;
    default:
        die(ERR_INTERN, "Unexpected breakpoint kind");
    }
}

/** Search for a breakpoint
 *
 * Search for a breakpoint in given list which should be
 * fired on given address and fire it.
 *
 * @param breakpoints List of code breakpoints of some processor.
 * @param address     Address of executed instruction.
 *
 * @return True, if at least one breakpoint has been hit.
 *
 */
static bool breakpoint_hit_by_address(list_t breakpoints, ptr64_t address)
{
    bool hit = false;

    breakpoint_t *breakpoint = NULL;
    for_each(breakpoints, breakpoint, breakpoint_t)
    {
        if (breakpoint->pc.ptr == address.ptr) {
            breakpoint_hit(breakpoint);
            hit = true;
        }
    }

    return hit;
}

/** Search for a breakpoint
 *
 * Search for a breakpoint in given list which should be
 * fired on given address and has kind which matches to the
 * given filter.
 *
 * @param breakpoints List of code breakpoints of some processor.
 * @param address     Address, where the breakpoint can be hit.
 * @param filter      Filters considered breakpoints according to the kind.
 *
 * @return Found breakpoint structure or NULL, if there is not any.
 *
 */
breakpoint_t *breakpoint_find_by_address(list_t breakpoints,
        ptr64_t address, breakpoint_filter_t filter)
{
    breakpoint_t *breakpoint = NULL;

    for_each(breakpoints, breakpoint, breakpoint_t)
    {
        if ((breakpoint->pc.ptr == address.ptr) && ((breakpoint->kind & filter) != 0)) {
            return breakpoint;
        }
    }

    return NULL;
}

/** Search all of the processors
 *
 * Search all of the processors whether any of them is going to
 * execute instruction where a code breakpoint is located. All such
 * breakpoints are fired.
 *
 * @return True, if at least one breakpoint has been fired.
 *
 */
bool breakpoint_check_for_code_breakpoints(void)
{
    // TODO: add SH2E support
    bool hit = false;
    device_t *dev = NULL;

    while (dev_next(&dev, DEVICE_FILTER_R4K_PROCESSOR)) {
        r4k_cpu_t *cpu = get_r4k(dev);

        if (breakpoint_hit_by_address(cpu->bps, cpu->pc)) {
            hit = true;
        }
    }

    if (hit) {
        return hit;
    }

    while (dev_next(&dev, DEVICE_FILTER_RV_PROCESSOR)) {
        const rv_cpu_t *cpu = get_rv(dev);

        ptr64_t addr = { 0 };
        addr.lo = cpu->pc;
        if (breakpoint_hit_by_address(cpu->bps, addr)) {
            hit = true;
        }
    }

    return hit;
}
