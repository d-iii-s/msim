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

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../assert.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "cpu/riscv_rv32ima/cpu.h"
#include "cpu/riscv_rv32ima/csr.h"
#include "cpu/riscv_rv32ima/debug.h"
#include "drvcpu.h"

static bool rv32_convert_add_wrapper(void *cpu, ptr64_t virt, ptr36_t *phys, bool write)
{
    // use only low 32-bits from virt
    return rv32_convert_addr((rv_cpu_t *) cpu, virt.lo, phys, write, false, false) == rv_exc_none;
}

static void rv32_set_pc_wrapper(void *cpu, ptr64_t addr)
{
    // use only low 32-bits from addr
    rv32_cpu_set_pc((rv_cpu_t *) cpu, addr.lo);
}

static const cpu_ops_t rv_cpu = {
    .interrupt_up = (interrupt_func_t) rv32_interrupt_up,
    .interrupt_down = (interrupt_func_t) rv32_interrupt_down,

    .convert_addr = (convert_addr_func_t) rv32_convert_add_wrapper,
    .reg_dump = (reg_dump_func_t) rv32_reg_dump,

    .set_pc = (set_pc_func_t) rv32_set_pc_wrapper,
    .sc_access = (sc_access_func_t) rv32_sc_access
};

/**
 * Initialization
 */
static bool drvcpu_init(token_t *parm, device_t *dev)
{

    unsigned int id = get_free_cpuno();

    if (id == MAX_CPUS) {
        error("Maximum CPU count exceeded (%u)", MAX_CPUS);
        return false;
    }

    rv32_cpu_t *cpu = safe_malloc_t(rv_cpu_t);
    rv32_cpu_init(cpu, id);
    general_cpu_t *gen_cpu = safe_malloc_t(general_cpu_t);
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
static bool drvcpu_info(token_t *parm, device_t *dev)
{
    printf("RV32IMA (processor ID: %i)\n", ((general_cpu_t *) dev->data)->cpuno);
    return true;
}

/**
 * RD command implementation
 */
static bool drvcpu_rd(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    rv32_reg_dump(get_rv(dev));
    return true;
}

/**
 * CSRD command implementation
 */
static bool drvcpu_csr_dump(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    if (parm->ttype == tt_end) {
        rv32_csr_dump_reduced(get_rv(dev));
        return true;
    }

    token_type_t token_type = parm_type(parm);

    if (token_type == tt_str) {
        const char *param = parm_str_next(&parm);

        if (rv32_csr_dump_command(get_rv(dev), param)) {
            return true;
        }

        if (rv32_csr_dump_by_name(get_rv(dev), param)) {
            return true;
        }

        return false;
    } else if (token_type == tt_uint) {
        uint64_t num = parm_uint_next(&parm);
        if (num > 0xFFF) {
            return false;
        }
        return rv32_csr_dump(get_rv(dev), num);
    }

    printf("Invalid arguments!");
    return false;
}

/**
 * TR command implementation
 */
static bool drvcpu_tr(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    uint64_t addr = parm_uint_next(&parm);

    if (addr > (uint64_t) UINT32_MAX) {
        error("Virtual memory address too large");
        return false;
    }

    return rv32_translate_dump(get_rv(dev), (uint32_t) addr);
}

/**
 * STR command implementation
 */
static bool drvcpu_str(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    ptr36_t root_phys = parm_uint_next(&parm);

    if (!IS_ALIGNED(root_phys, RV_PAGEBYTES)) {
        error("Root pagetable physical address 0x%09" PRIx64 " is not aligned to pagesize!", root_phys);
        return false;
    }

    uint64_t addr = parm_uint_next(&parm);

    if (addr > (uint64_t) UINT32_MAX) {
        error("Virtual memory address too large");
        return false;
    }

    return rv32_translate_sv32_dump(get_rv(dev), root_phys, (uint32_t) addr);
}

static bool drvcpu_specifies_verbose(const char *param)
{
    return strcmp(param, "v") == 0 || strcmp(param, "verbose") == 0;
}

/**
 * PTD command implementation
 */
static bool drvcpu_ptd(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    if (parm->ttype == tt_end) {
        return rv32_pagetable_dump(get_rv(dev), false);
    }

    const char *param = parm_str_next(&parm);

    if (!drvcpu_specifies_verbose(param)) {
        error("Invalid parameter <%s>", param);
        return false;
    }

    return rv32_pagetable_dump(get_rv(dev), true);
}

/**
 * SPTD command implementation
 */
static bool drvcpu_sptd(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    ptr36_t root_phys = parm_uint_next(&parm);

    if (!IS_ALIGNED(root_phys, RV_PAGEBYTES)) {
        error("Root pagetable physical address 0x%09" PRIx64 " is not aligned to pagesize!", root_phys);
        return false;
    }

    if (parm->ttype == tt_end) {
        return rv32_pagetable_dump_from_phys(get_rv(dev), root_phys, false);
    }

    const char *param = parm_str_next(&parm);

    if (!drvcpu_specifies_verbose(param)) {
        error("Invalid parameter <%s>", param);
        return false;
    }

    return rv32_pagetable_dump_from_phys(get_rv(dev), root_phys, true);
}

/**
 * TLBD command implementation
 */
static bool drvcpu_tlb_dump(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);
    rv32_tlb_dump(&get_rv(dev)->tlb);
    return true;
}

/**
 * TLBRESIZE command implementation
 */
static bool drvcpu_tlb_resize(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    size_t new_tlb_size = parm_uint_next(&parm);

    if (new_tlb_size == 0) {
        error("TLB size cannot be 0!\n");
        return false;
    }

    rv32_tlb_t *tlb = &get_rv(dev)->tlb;

    return rv32_tlb_resize(tlb, new_tlb_size);
}

static bool drvcpu_tlb_flush(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    rv32_tlb_t *tlb = &get_rv(dev)->tlb;

    rv32_tlb_flush(tlb);

    return true;
}

/**
 * SETASIDLEN command implementation
 */
static bool drvcpu_set_asid_len(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    size_t new_asid_len = parm_uint_next(&parm);

    if (new_asid_len > rv_asid_len) {
        error("Number of bits of ASID cannot exceed 9!\n");
        return false;
    }

    rv32_csr_set_asid_len(get_rv(dev), new_asid_len);

    return true;
}

/**
 * Done device operation
 */
static void drvcpu_done(device_t *dev)
{
    rv32_cpu_done(get_rv(dev));
    safe_free(((general_cpu_t *) dev->data)->data);
    safe_free(dev->data)
}

/**
 * Step device operation
 */
static void drvcpu_step(device_t *dev)
{
    rv32_cpu_step(get_rv(dev));
}

/**
 * Device commands specification
 */
cmd_t drvcpu_cmds[] = {
    { "init",
            (fcmd_t) drvcpu_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "pname/processor name" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display this help text",
            "Display this help text",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) drvcpu_info,
            DEFAULT,
            DEFAULT,
            "Display configuration information",
            "Display configuration information",
            NOCMD },
    { "rd",
            (fcmd_t) drvcpu_rd,
            DEFAULT,
            DEFAULT,
            "Dump content of CPU general registers",
            "Dump content of CPU general registers",
            NOCMD },
    { "csrd",
            (fcmd_t) drvcpu_csr_dump,
            DEFAULT,
            DEFAULT,
            "Dump content of CSR registers",
            "Dump content of some CSRs if no argument is given, dump the content of the specified register (numerically or by name), or dump a predefined set of CSRs (mmode, smode, counters or all)",
            OPT VAR "csr" END },
    { "tr",
            (fcmd_t) drvcpu_tr,
            DEFAULT,
            DEFAULT,
            "Translates the specified address",
            "Translates the specified virtual address based on the current CPU state and describes the translation steps.",
            REQ INT "addr/virtual address" END },
    { "str",
            (fcmd_t) drvcpu_str,
            DEFAULT,
            DEFAULT,
            "Translates the specified address using the given pagetable",
            "Translates the specified virtual address using the pagetable specified by the physical address of its root pagetable and describes the translation steps.",
            REQ INT "phys/root pagetable physical address" NEXT
                    REQ INT "addr/virtual address" END },
    { "ptd",
            (fcmd_t) drvcpu_ptd,
            DEFAULT,
            DEFAULT,
            "Dumps the current pagetable",
            "Prints out all valid PTEs in the pagetable currently pointed to by the satp CSR. Adding the `verbose` parameter prints out all nonzero PTEs.",
            OPT STR "verbose" END },
    { "sptd",
            (fcmd_t) drvcpu_sptd,
            DEFAULT,
            DEFAULT,
            "Dumps the specified pagetable",
            "Prints out all valid PTEs in the pagetable with its root on the specified physical address. Adding the `verbose` parameter prints out all nonzero PTEs.",
            REQ INT "phys/physical address of the root pagetable" NEXT
                    OPT STR "verbose" END },
    { "tlbd",
            (fcmd_t) drvcpu_tlb_dump,
            DEFAULT,
            DEFAULT,
            "Dump valid content of the TLB",
            "Dump content of the TLB. Dumps only valid entries.",
            NOCMD },
    { "tlbresize",
            (fcmd_t) drvcpu_tlb_resize,
            DEFAULT,
            DEFAULT,
            "Resize the TLB",
            "Resizes the TLB, flushing it completely in the process.",
            REQ INT "TLB size" END },
    { "tlbflush",
            (fcmd_t) drvcpu_tlb_flush,
            DEFAULT,
            DEFAULT,
            "Flushes the TLB",
            "Removes all entries from the TLB.",
            NOCMD },
    { "asidlen",
            (fcmd_t) drvcpu_set_asid_len,
            DEFAULT,
            DEFAULT,
            "Changes the bit-length of ASIDs",
            "Changes the number of usable bits in the ASID field of the SATP CSR, zeroes-out any deactivated bits and flushes the TLB.",
            REQ INT "ASID length" END }
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
