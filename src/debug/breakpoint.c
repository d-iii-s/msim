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

#include "breakpoint.h"

#include <inttypes.h>

#include "../utils.h"
#include "../check.h"
#include "../io/output.h"
#include "../device/machine.h"
#include "gdb.h"

list_t memory_breakpoints;

/************************************************************************/
/* Memory breakpoints                                                   */
/************************************************************************/

/** Routine for initializing global variables in this module */
void memory_breakpoint_init_framework(void)
{
	list_init(&memory_breakpoints);
}

/** Allocate and initialize a memory breakpoint
 *
 * @param address      Address, where the breakpoint can be hit.
 * @param kind         Specifies if the breakpoint was initiated for the simulator of
 *                     from the debugger.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 * @return Initialized memory breakpoint structure.
 *
 */
static mem_breakpoint_t *memory_breakpoint_init(ptr_t address,
    breakpoint_kind_t kind, access_filter_t access_flags)
{
	mem_breakpoint_t *breakpoint =
	    (mem_breakpoint_t *) safe_malloc_t(mem_breakpoint_t);
	
	item_init(&breakpoint->item);
	breakpoint->kind = kind;
	breakpoint->addr = address;
	breakpoint->hits = 0;
	breakpoint->access_flags = access_flags;
	
	return breakpoint;
}

/** Setup and activate a new memory breakpoint
 *
 * @param address      Address, where the breakpoint can be hit.
 * @param kind         Specifies if the breakpoint was initiated for the simulator of
 *                     from the debugger.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 */
void memory_breakpoint_add(ptr_t address, breakpoint_kind_t kind,
    access_filter_t access_flags)
{
	mem_breakpoint_t *breakpoint =
	    memory_breakpoint_init(address, kind, access_flags);
	
	list_append(&memory_breakpoints, &breakpoint->item);
}

/** Deactivate memory breakpoint with specified address
 *
 * @param address Address, where the breakpoint can be hit.
 *
 * @return True, if some breakpoint has been deactivated.
 *
 */
bool memory_breakpoint_remove(ptr_t address)
{
	mem_breakpoint_t *breakpoint =
	    (mem_breakpoint_t *) memory_breakpoints.head;
	
	while (breakpoint != NULL) {
		if (breakpoint->addr == address) {
			list_remove(&memory_breakpoints, &breakpoint->item);
			safe_free(breakpoint);
			
			return true;
		}
		
		breakpoint = (mem_breakpoint_t *) breakpoint->item.next;
	}
	
	return false;
}

/** Deactivate all the memory breakpoint, which matches to the given filter.
 *
 * @param filter Filter for selecting breakpoints, which will be deactivated.
 *
 */
void memory_breakpoint_remove_filtered(breakpoint_filter_t filter)
{
	mem_breakpoint_t *breakpoint =
	    (mem_breakpoint_t *) memory_breakpoints.head;
	
	while (breakpoint != NULL) {
		mem_breakpoint_t *removed = breakpoint;
		breakpoint = (mem_breakpoint_t *) breakpoint->item.next;
		
		/* Ignore breakpoints, which are not filtered */
		if ((removed->kind & filter) == 0)
			continue;
		
		list_remove(&memory_breakpoints, &removed->item);
		safe_free(removed);
	}
}

/** Print activated memory breakpoints for the user */
void memory_breakpoint_print_list(void)
{
	mprintf("Address    Mode Hits\n");
	mprintf("---------- ---- --------------------\n");
	
	mem_breakpoint_t *breakpoint = NULL;
	for_each(memory_breakpoints, breakpoint, mem_breakpoint_t) {
		bool read = breakpoint->access_flags & ACCESS_READ;
		bool write = breakpoint->access_flags & ACCESS_WRITE;
		
		mprintf("%#010" PRIx32 " %c%c   %20" PRIu64 "\n",
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
void memory_breakpoint_hit(mem_breakpoint_t *breakpoint, access_t access_type)
{
	PRE(breakpoint != NULL);
	
	switch (breakpoint->kind) {
	case BREAKPOINT_KIND_SIMULATOR:
		if (access_type == ACCESS_READ)
			mprintf("\nDebug: Read from address %#10" PRIx32 "\n\n",
			    breakpoint->addr);
		else
			mprintf("\nDebug: Written to address %#10" PRIx32 "\n\n",
			    breakpoint->addr);
		
		breakpoint->hits++;
		interactive = true;
		break;
	case BREAKPOINT_KIND_DEBUGGER:
		gdb_handle_event(GDB_EVENT_BREAKPOINT);
		break;
	default:
		PRE(false);
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
breakpoint_t *breakpoint_init(ptr_t address, breakpoint_kind_t kind)
{
	breakpoint_t *breakpoint =
	    (breakpoint_t *) safe_malloc_t(breakpoint_t);
	
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
		mprintf("\nDebug: Hit breakpoint at %08x\n\n", breakpoint->pc);
		interactive = true;
		break;
	case BREAKPOINT_KIND_DEBUGGER:
		gdb_handle_event(GDB_EVENT_BREAKPOINT);
		break;
	default:
		PRE(false);
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
static bool breakpoint_hit_by_address(list_t breakpoints, ptr_t address)
{
	bool hit = false;
	
	breakpoint_t *breakpoint = NULL;
	for_each (breakpoints, breakpoint, breakpoint_t) {
		if (breakpoint->pc == address) {
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
    ptr_t address, breakpoint_filter_t filter)
{
	breakpoint_t *breakpoint = NULL;
	
	for_each (breakpoints, breakpoint, breakpoint_t) {
		if ((breakpoint->pc == address) &&
		    ((breakpoint->kind & filter) != 0))
				return breakpoint;
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
	bool hit = false;
	device_s *dev = NULL;
	
	while (dev_next(&dev, DEVICE_FILTER_PROCESSOR)) {
		processor_t* processor = (processor_t *) dev->data;
		
		if (breakpoint_hit_by_address(processor->bps, processor->pc))
			hit = true;
	}
	
	return hit;
}
