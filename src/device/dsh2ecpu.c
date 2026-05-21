/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../assert.h"
#include "../debug/breakpoint.h"
#include "../debug/debug.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "cpu/superh_sh2e/cpu.h"
#include "cpu/superh_sh2e/debug.h"
#include "device.h"
#include "dsh2ecmt.h"
#include "dsh2ecpu.h"

#define device_get_sh2e_cpu(dev) (sh2e_cpu_t *) (((general_cpu_t *) (dev)->data)->data)

static cpu_ops_t const sh2e_cpu_ops = {
    .convert_addr = (convert_addr_func_t) sh2e_cpu_convert_addr,
    .reg_dump = (reg_dump_func_t) sh2e_cpu_dump_cpu_regs,
    .set_pc = (set_pc_func_t) sh2e_cpu_goto,

    // Interrupts
    .interrupt_up = (interrupt_func_t) sh2e_cpu_assert_interrupt,
    .interrupt_down = (interrupt_func_t) sh2e_cpu_deassert_interrupt,
};

/** Processor initialization. */
static bool
dsh2ecpu_cmd_init(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    // TODO Functions to create generic CPU should be provided by 'cpu/generic.h'
    unsigned int id = get_free_cpuno();
    if (id == MAX_CPUS) {
        error("Maximum CPU count exceeded (%u)", MAX_CPUS);
        return false;
    }

    sh2e_cpu_t *sh2e_cpu = safe_malloc_t(sh2e_cpu_t);
    sh2e_cpu_init(sh2e_cpu, id);

    general_cpu_t *generic_cpu = safe_malloc_t(general_cpu_t);
    *generic_cpu = (general_cpu_t) {
        .cpuno = id,
        .data = sh2e_cpu,
        .type = &sh2e_cpu_ops
    };

    add_cpu(generic_cpu);

    dev->data = generic_cpu;
    return true;
}

/** Processor cleanup. */
static void
dsh2ecpu_done(device_t *const dev)
{
    ASSERT(dev != NULL);

    general_cpu_t *generic_cpu = (general_cpu_t *) dev->data;

    sh2e_cpu_t *sh2e_cpu = (sh2e_cpu_t *) generic_cpu->data;
    sh2e_cpu_done(sh2e_cpu);
    safe_free(sh2e_cpu);

    safe_free(generic_cpu);
}

/** Processor info command. */
static bool
dsh2ecpu_cmd_info(token_t *const parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    printf("SH-2E\n");
    return true;
}

/** Dump memory (data) command. */
static bool
dsh2ecpu_cmd_dump_data(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    // TODO Allow using register names (or numbers) instead of address.
    uint32_t addr = ALIGN_DOWN(parm_uint_next(&parm), sizeof(uint32_t));

    // TODO Use a sensible default if missing.
    uint32_t count = parm_uint(parm);

    sh2e_cpu_dump_data(device_get_sh2e_cpu(dev), addr, count);
    return true;
}

/** Dump memory (instructions) command. */
static bool
dsh2ecpu_cmd_dump_code(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    // TODO Allow using register names (or numbers) instead of address.
    uint32_t addr = ALIGN_DOWN(parm_uint_next(&parm), sizeof(sh2e_insn_t));

    // TODO Use a sensible default if missing.
    uint32_t count = parm_uint(parm);

    sh2e_cpu_dump_code(device_get_sh2e_cpu(dev), addr, count);
    return true;
}

/** Dump CPU registers command. */
static bool
dsh2ecpu_cmd_dump_cpu_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_cpu_dump_cpu_regs(device_get_sh2e_cpu(dev));
    return true;
}

/** Dump FPU registers command. */
static bool
dsh2ecpu_cmd_dump_fpu_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_cpu_dump_fpu_regs(device_get_sh2e_cpu(dev));
    return true;
}

/** Goto command. */
static bool
dsh2ecpu_cmd_goto(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    // TODO Maybe just use uint32_t for address?
    ptr64_t addr = { .ptr = ALIGN_DOWN(parm_uint_next(&parm), sizeof(sh2e_insn_t)) };
    sh2e_cpu_goto(device_get_sh2e_cpu(dev), addr);
    return true;
}

/** Stat command implementation
 *
 */
static bool dsh2ecpu_cmd_stat(token_t *parm, device_t *dev)
{
    sh2e_cpu_t *cpu = device_get_sh2e_cpu(dev);

    printf("[Total cycles            ] [Program execution cycles] [Power down cycles       ]\n");
    printf("%26" PRIu64 " %26" PRIu64 " %26" PRIu64 "\n\n",
            (uint64_t) cpu->program_execution_cycles + cpu->power_down_cycles,
            cpu->program_execution_cycles, cpu->power_down_cycles);

    return true;
}

/**
 * Assert an interrupt. (using this in order to mimick a device interrupt without acctually having a device in configuration)
 */
static bool dsh2ecpu_cmd_assert_interrupt(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    uint16_t interrupt_source = parm_uint_next(&parm);

    sh2e_cpu_assert_interrupt(device_get_sh2e_cpu(dev), interrupt_source);

    return true;
}

static bool dsh2ecpu_cmd_set_intc(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    const char *intc_name = parm_str_next(&parm);
    device_t *intc_dev = dev_by_name(intc_name);

    if (intc_dev == NULL) {
        error("Interrupt controller device '%s' not found", intc_name);
        return false;
    }

    sh2e_cpu_t *cpu = device_get_sh2e_cpu(dev);

    cpu->intc = get_generic_intc(intc_dev);
    if (cpu->intc == NULL) {
        error("Failed to get SH-2E interrupt controller from device '%s'", intc_name);
        return false;
    }

    return true;
}

static bool dsh2ecpu_cmd_add_cmt(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    const char *cmt_name = parm_str_next(&parm);
    device_t *cmt_dev = dev_by_name(cmt_name);

    if (cmt_dev == NULL) {
        error("Compare match timer device '%s' not found", cmt_name);
        return false;
    }

    sh2e_cpu_t *cpu = device_get_sh2e_cpu(dev);
    sh2e_cmt_t *cmt = device_get_sh2e_cmt(cmt_dev);

    list_append(&cpu->on_chip_peripherals.cmt_list, &cmt->item);

    return true;
}

/** Execute one processor step. */
static void
dsh2ecpu_step(device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_cpu_step(device_get_sh2e_cpu(dev));
}

static cmd_t const dsh2ecpu_cmds[] = {
    { "init",
            (fcmd_t) dsh2ecpu_cmd_init,
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
            (fcmd_t) dsh2ecpu_cmd_info,
            DEFAULT,
            DEFAULT,
            "Display configuration information",
            "Display configuration information",
            NOCMD },
    { "md",
            (fcmd_t) dsh2ecpu_cmd_dump_data,
            DEFAULT,
            DEFAULT,
            "Dump specified physical memory block",
            "Dump specified physical memory block",
            REQ INT "saddr/starting address" NEXT
                    REQ INT "size/size" END },
    { "id",
            (fcmd_t) dsh2ecpu_cmd_dump_code,
            DEFAULT,
            DEFAULT,
            "Dump instructions from physical memory location",
            "Dump instructions from physical memory location",
            REQ INT "saddr/starting address" NEXT
                    REQ INT "cnt/count" END },
    { "rd",
            (fcmd_t) dsh2ecpu_cmd_dump_cpu_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of CPU registers",
            "Dump contents of CPU registers",
            NOCMD },
    { "frd",
            (fcmd_t) dsh2ecpu_cmd_dump_fpu_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of FPU registers",
            "Dump contents of FPU registers",
            NOCMD },
    { "goto",
            (fcmd_t) dsh2ecpu_cmd_goto,
            DEFAULT,
            DEFAULT,
            "Go to address",
            "Go to address",
            REQ INT "addr/address" END },
    { "stat",
            (fcmd_t) dsh2ecpu_cmd_stat,
            DEFAULT,
            DEFAULT,
            "Display CPU statistics",
            "Display CPU statistics",
            NOCMD },
    { "assertint",
            (fcmd_t) dsh2ecpu_cmd_assert_interrupt,
            DEFAULT,
            DEFAULT,
            "Assert interrupt",
            "Assert interrupt <interrupt_source>",
            REQ INT "interrupt_source" END },
    { "setintc",
            (fcmd_t) dsh2ecpu_cmd_set_intc,
            DEFAULT,
            DEFAULT,
            "Set interrupt controller",
            "Set interrupt controller <intc_device_name>",
            REQ STR "intc_device_name" END },
    { "addcmt",
            (fcmd_t) dsh2ecpu_cmd_add_cmt,
            DEFAULT,
            DEFAULT,
            "Add a compare match timer to the CPU's on-chip peripherals",
            "Add a compare match timer to the CPU's on-chip peripherals <cmt_device_name>",
            REQ STR "cmt_device_name" END },
    LAST_CMD
};

device_type_t const dsh2ecpu = {
    /* CPU is simulated deterministically */
    .nondet = false,

    /* Device type name. */
    .name = "dsh2ecpu",

    /* Device brief description. */
    .brief = "SuperH SH-2E processor",

    /* Device full description. */
    .full = "SuperH SH-2E 32-bit processor (with FPU)",

    /* Device functions. */
    .done = dsh2ecpu_done,
    .step = dsh2ecpu_step,

    /* Commands */
    .cmds = dsh2ecpu_cmds,
};
