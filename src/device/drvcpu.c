/*
 * Copyright (c) 2004-2007 Viliam Holub
 * Copyright (c) 2008-2011 Martin Decky
 * Copyright (c) 2022   Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V RV32IMA device
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "drvcpu.h"
#include "cpu/general_cpu.h"
#include "cpu/riscv_rv32ima/cpu.h"
#include "cpu/riscv_rv32ima/debug.h"
#include "../main.h"
#include "../assert.h"
#include "../utils.h"
#include "../fault.h"

static bool rv_convert_add_wrapper(void* cpu, ptr64_t virt,  ptr36_t* phys, bool write){
    // use only low 32-bits from virt
    return rv_convert_addr((rv_cpu_t*)cpu, virt.lo, phys, write, false, false) == rv_exc_none;
}

static void rv_set_pc_wrapper(void* cpu, ptr64_t addr){
    // use only low 32-bits from addr
    rv_cpu_set_pc((rv_cpu_t*)cpu, addr.lo);
}

static const cpu_ops_t rv_cpu = {
	.interrupt_up = (interrupt_func_t)rv_interrupt_up,
	.interrupt_down = (interrupt_func_t)rv_interrupt_down,
	
	.convert_addr = (convert_addr_func_t)rv_convert_add_wrapper,
    .reg_dump = (reg_dump_func_t)rv_reg_dump,
    
	.set_pc = (set_pc_func_t)rv_set_pc_wrapper,
	.sc_access = (sc_access_func_t)rv_sc_access
};

/**
 * Initialization
 */
static bool drvcpu_init(token_t *parm, device_t *dev){
    
    unsigned int id = get_free_cpuno();

    if(id == MAX_CPUS) {
        error("Maximum CPU count exceeded (%u)", MAX_CPUS);
		return false;
    }

    rv_cpu_t *cpu = safe_malloc_t(rv_cpu_t);
    rv_cpu_init(cpu, id);
    general_cpu_t* gen_cpu = safe_malloc_t(general_cpu_t);
    gen_cpu->cpuno = id;
    gen_cpu->data = cpu;
    gen_cpu->type = &rv_cpu;

    add_cpu(gen_cpu);

    dev->data = gen_cpu;

    return true;
}

/**
 * Info command implementation
 */
static bool drvcpu_info(token_t *parm, device_t *dev){
    printf("RV32IMA (processor ID: %i)\n", ((general_cpu_t *)dev->data)->cpuno);
    return true;
}

/**
 * RD command implementation
 */
static bool drvcpu_rd(token_t *parm, device_t *dev){
    ASSERT(dev != NULL);

    rv_reg_dump(get_rv(dev));
    return true;
}

/**
 * CSRRD command implementation
 */
static bool drvcpu_csr_rd(token_t *parm, device_t *dev){
    ASSERT(dev != NULL);

    if(parm->ttype == tt_end) {
        rv_csr_dump_all(get_rv(dev));
        return true;
    }

    token_type_t token_type = parm_type(parm);

    if(token_type == tt_str){
        const char* name = parm_str_next(&parm);
        return rv_csr_dump_by_name(get_rv(dev), name);
    }
    else if(token_type == tt_uint){
        uint64_t num = parm_uint_next(&parm);
        if(num > 0xFFF) return false;
        return rv_csr_dump(get_rv(dev), num);
    }

    printf("Invalid arguments!");
    return false;
}

/**
 * TLBRD command implementation
 */
static bool drvcpu_tlb_rd(token_t *parm, device_t *dev){
    ASSERT(dev != NULL);
    rv_tlb_dump(&get_rv(dev)->tlb);
    return true;
}

/**
 * TLBRESIZE command implementation
 */
static bool drvcpu_tlb_resize(token_t *parm, device_t *dev){
    ASSERT(dev != NULL);
    
    size_t new_tlb_size = parm_uint_next(&parm);

    if(new_tlb_size == 0){
        error("TLB size cannot be 0!\n");
        return false;
    }

    rv_tlb_t* tlb = &get_rv(dev)->tlb;

    return rv_tlb_resize(tlb, new_tlb_size);
}

/**
 * Done device operation
 */
static void drvcpu_done(device_t *dev){
    rv_cpu_done(get_rv(dev));
    safe_free(((general_cpu_t *)dev->data)->data);
    safe_free(dev->data)
}

/**
 * Step device operation
 */
static void drvcpu_step(device_t *dev){
    rv_cpu_step(get_rv(dev));
}

/**
 * Device commands specification
 */
cmd_t drvcpu_cmds[] = {
    {
        "init",
        (fcmd_t) drvcpu_init,
        DEFAULT,
        DEFAULT,
        "Initialization",
        "Initialization",
        REQ STR "pname/processor name" END
    },
    {
        "help",
        (fcmd_t) dev_generic_help,
        DEFAULT,
        DEFAULT,
        "Display this help text",
        "Display this help text",
        OPT STR "cmd/command name" END
    },
    {
        "info",
        (fcmd_t) drvcpu_info,
        DEFAULT,
        DEFAULT,
        "Display configuration information",
        "Display configuration information",
        NOCMD
    },
    {
        "rd",
        (fcmd_t) drvcpu_rd,
        DEFAULT,
        DEFAULT,
        "Dump content of CPU general registers",
        "Dump content of CPU general registers",
        NOCMD
    },
    {
        "csrrd",
        (fcmd_t) drvcpu_csr_rd,
        DEFAULT,
        DEFAULT,
        "Dump content of CSR registers",
        "Dump content of all CSRs if no argument is given, or dump the content of the specified register (numerically or by name)",
        OPT VAR "csr" END
    },
    {
        "tlbrd",
        (fcmd_t) drvcpu_tlb_rd,
        DEFAULT,
        DEFAULT,
        "Dump valid content of the TLB",
        "Dump content of the TLB. Dumps only valid entries.",
        NOCMD
    },
    {
        "tlbresize",
        (fcmd_t) drvcpu_tlb_resize,
        DEFAULT,
        DEFAULT,
        "Resize the TLB",
        "Resizes the TLB, flushing it completely in the process.",
        REQ INT "TLB size" END
    }
};

/**
 * Device type specification
 */
device_type_t drvcpu = {
    .nondet = false,

    .name = "drvcpu",

    .brief = "RISC-V RV32IMA processor",

    .full = "RISC-V processor, supporting M and A extensions, with support for Machine, Supervisor and User mode and with virtual memory support.",

    .done = drvcpu_done,
    .step = drvcpu_step,

    .cmds = drvcpu_cmds
};