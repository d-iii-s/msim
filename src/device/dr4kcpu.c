/*
 * Copyright (c) 2003-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS R4000 microprocessor (32 bit part) device
 *
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../debug/breakpoint.h"
#include "../debug/debug.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "cpu/mips_r4000/cpu.h"
#include "cpu/mips_r4000/debug.h"
#include "device.h"
#include "dr4kcpu.h"

static bool r4k_cpu_convert_addr(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write)
{
    return r4k_convert_addr(cpu, virt, phys, write, false) == r4k_excNone;
}

static const cpu_ops_t r4k_cpu = {
    .interrupt_up = (interrupt_func_t) r4k_interrupt_up,
    .interrupt_down = (interrupt_func_t) r4k_interrupt_down,

    .convert_addr = (convert_addr_func_t) r4k_cpu_convert_addr,
    .reg_dump = (reg_dump_func_t) r4k_reg_dump,
    .set_pc = (set_pc_func_t) r4k_set_pc,
    .sc_access = (sc_access_func_t) r4k_sc_access
};

/** Initialization
 *
 */
static bool dr4kcpu_init(token_t *parm, device_t *dev)
{
    unsigned int id = get_free_cpuno();

    if (id == MAX_CPUS) {
        error("Maximum CPU count exceeded (%u)", MAX_CPUS);
        return false;
    }

    r4k_cpu_t *cpu = safe_malloc_t(r4k_cpu_t);
    r4k_init(cpu, id);
    general_cpu_t *gen_cpu = safe_malloc_t(general_cpu_t);
    gen_cpu->cpuno = id;
    gen_cpu->data = cpu;
    gen_cpu->type = &r4k_cpu;

    add_cpu(gen_cpu);

    dev->data = gen_cpu;

    return true;
}

/** Info command implementation
 *
 */
static bool dr4kcpu_info(token_t *parm, device_t *dev)
{
    printf("R4000\n");
    return true;
}

/** Stat command implementation
 *
 */
static bool dr4kcpu_stat(token_t *parm, device_t *dev)
{
    r4k_cpu_t *cpu = get_r4k(dev);

    printf("[Total cycles      ] [In kernel space   ] [In user space     ]\n");
    printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
            (uint64_t) cpu->k_cycles + cpu->u_cycles + cpu->w_cycles,
            cpu->k_cycles, cpu->u_cycles);

    printf("[Wait cycles       ] [TLB Refill exc    ] [TLB Invalid exc   ]\n");
    printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
            cpu->w_cycles, cpu->tlb_refill, cpu->tlb_invalid);

    printf("[TLB Modified exc  ] [Interrupt 0       ] [Interrupt 1       ]\n");
    printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
            cpu->tlb_modified, cpu->intr[0], cpu->intr[1]);

    printf("[Interrupt 2       ] [Interrupt 3       ] [Interrupt 4       ]\n");
    printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
            cpu->intr[2], cpu->intr[3], cpu->intr[4]);

    printf("[Interrupt 5       ] [Interrupt 6       ] [Interrupt 7       ]\n");
    printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n",
            cpu->intr[5], cpu->intr[6], cpu->intr[7]);

    return true;
}

/** Cp0d command implementation
 *
 */
static bool dr4kcpu_cp0d(token_t *parm, device_t *dev)
{
    if (parm->ttype == tt_end) {
        r4k_cp0_dump_all(get_r4k(dev));
        return true;
    }
    if (parm->ttype != tt_uint) {
        error("Register number required");
        return false;
    }
    uint64_t no = parm_uint(parm);
    if (no >= MAX_CP0_REGISTERS) {
        error("Out of range (0..%u)", MAX_CP0_REGISTERS - 1);
        return false;
    }

    r4k_cp0_dump(get_r4k(dev), no);
    return true;
}

/** Tlbd command implementation
 *
 */
static bool dr4kcpu_tlbd(token_t *parm, device_t *dev)
{
    r4k_tlb_dump(get_r4k(dev));
    return true;
}

/** Md command implementation
 *
 */
static bool dr4kcpu_md(token_t *parm, device_t *dev)
{
    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
    uint64_t _cnt = parm_uint(parm);

    if (!virt_range(_addr)) {
        error("Virtual address out of range");
        return false;
    }

    if (!virt_range(_cnt)) {
        error("Count out of virtual memory range");
        return false;
    }

    if (!virt_range(_addr + _cnt * 4)) {
        error("Count exceeds virtual memory range");
        return false;
    }

    ptr64_t addr;
    len64_t cnt;
    len64_t i;

    for (addr.ptr = _addr, cnt = _cnt, i = 0;
            i < cnt; addr.ptr += 4, i++) {
        if ((i & 0x03U) == 0)
            printf("  %#018" PRIx64 "    ", addr.ptr);

        uint32_t val;
        r4k_exc_t res = r4k_read_mem32(get_r4k(dev), addr, &val, false);

        if (res == r4k_excNone)
            printf("%08" PRIx32 " ", val);
        else
            printf("xxxxxxxx ");

        if ((i & 0x03U) == 3)
            printf("\n");
    }

    if (i != 0)
        printf("\n");

    return true;
}

/** Id command implementation
 *
 */
static bool dr4kcpu_id(token_t *parm, device_t *dev)
{
    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
    uint64_t _cnt = parm_uint(parm);

    if (!virt_range(_addr)) {
        error("Virtual address out of range");
        return false;
    }

    if (!virt_range(_cnt)) {
        error("Count out of virtual memory range");
        return false;
    }

    if (!virt_range(_addr + _cnt * 4)) {
        error("Count exceeds virtual memory range");
        return false;
    }

    ptr64_t addr;
    len64_t cnt;

    for (addr.ptr = _addr, cnt = _cnt; cnt > 0;
            addr.ptr += 4, cnt--) {
        r4k_instr_t instr;
        // FIXME
        r4k_exc_t res = r4k_excNone;
        // exc_t res = cpu_read_ins((r4k_cpu_t *) dev->data->data, addr, &instr.val, false);

        if (res != r4k_excNone)
            instr.val = 0;

        r4k_idump(get_r4k(dev), addr, instr, false);
    }

    return true;
}

/** Rd command implementation
 *
 */
static bool dr4kcpu_rd(token_t *parm, device_t *dev)
{
    r4k_reg_dump(get_r4k(dev));
    return true;
}

/** Goto command implementation
 *
 */
static bool dr4kcpu_goto(token_t *parm, device_t *dev)
{
    r4k_cpu_t *cpu = get_r4k(dev);
    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);

    if (!virt_range(_addr)) {
        error("Virtual address out of range");
        return false;
    }

    ptr64_t addr;
    addr.ptr = _addr;

    r4k_set_pc(cpu, addr);
    return true;
}

/** Break command implementation
 *
 */
static bool dr4kcpu_break(token_t *parm, device_t *dev)
{
    r4k_cpu_t *cpu = get_r4k(dev);
    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);

    if (!virt_range(_addr)) {
        error("Virtual address out of range");
        return false;
    }

    ptr64_t addr;
    // Extend the address as the user will not enter it in 64bit mode
    // when the emulated CPU is 32bit.
    addr.ptr = UINT64_C(0xffffffff00000000) | _addr;

    breakpoint_t *bp = breakpoint_init(addr,
            BREAKPOINT_KIND_SIMULATOR);
    list_append(&cpu->bps, &bp->item);

    return true;
}

/** Bd command implementation
 *
 */
static bool dr4kcpu_bd(token_t *parm, device_t *dev)
{
    r4k_cpu_t *cpu = get_r4k(dev);
    breakpoint_t *bp;

    printf("[address ] [hits              ] [kind    ]\n");

    for_each(cpu->bps, bp, breakpoint_t)
    {
        const char *kind = (bp->kind == BREAKPOINT_KIND_SIMULATOR)
                ? "Simulator"
                : "Debugger";

        printf("%#018" PRIx64 " %20" PRIu64 " %s\n",
                bp->pc.ptr, bp->hits, kind);
    }

    return true;
}

/** Br command implementation
 *
 */
static bool dr4kcpu_br(token_t *parm, device_t *dev)
{
    r4k_cpu_t *cpu = get_r4k(dev);
    uint64_t addr = ALIGN_DOWN(parm_uint_next(&parm), 4);

    if (!virt_range(addr)) {
        error("Virtual address out of range");
        return false;
    }

    bool fnd = false;
    breakpoint_t *bp;
    for_each(cpu->bps, bp, breakpoint_t)
    {
        if (bp->pc.ptr == addr) {
            list_remove(&cpu->bps, &bp->item);
            safe_free(bp);
            fnd = true;
            break;
        }
    }

    if (!fnd) {
        error("Unknown breakpoint");
        return false;
    }

    return true;
}

/** Done
 *
 */
static void dr4kcpu_done(device_t *dev)
{
    r4k_done(get_r4k(dev));
    safe_free(((general_cpu_t *) dev->data)->data);
    safe_free(dev->data);
}

/** Execute one processor step
 *
 */
static void dr4kcpu_step(device_t *dev)
{
    r4k_step(get_r4k(dev));
}

cmd_t dr4kcpu_cmds[] = {
    { "init",
            (fcmd_t) dr4kcpu_init,
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
            (fcmd_t) dr4kcpu_info,
            DEFAULT,
            DEFAULT,
            "Display configuration information",
            "Display configuration information",
            NOCMD },
    { "stat",
            (fcmd_t) dr4kcpu_stat,
            DEFAULT,
            DEFAULT,
            "Display processor statistics",
            "Display processor statistics",
            NOCMD },
    { "cp0d",
            (fcmd_t) dr4kcpu_cp0d,
            DEFAULT,
            DEFAULT,
            "Dump contents of the coprocessor 0 register(s)",
            "Dump contents of the coprocessor 0 register(s)",
            OPT INT "rn/register number" END },
    { "tlbd",
            (fcmd_t) dr4kcpu_tlbd,
            DEFAULT,
            DEFAULT,
            "Dump content of TLB",
            "Dump content of TLB",
            NOCMD },
    { "md",
            (fcmd_t) dr4kcpu_md,
            DEFAULT,
            DEFAULT,
            "Dump specified TLB mapped memory block",
            "Dump specified TLB mapped memory block",
            REQ INT "saddr/starting address" NEXT
                    REQ INT "size/size" END },
    { "id",
            (fcmd_t) dr4kcpu_id,
            DEFAULT,
            DEFAULT,
            "Dump instructions from specified TLB mapped memory",
            "Dump instructions from specified TLB mapped memory",
            REQ INT "saddr/starting address" NEXT
                    REQ INT "cnt/count" END },
    { "rd",
            (fcmd_t) dr4kcpu_rd,
            DEFAULT,
            DEFAULT,
            "Dump contents of CPU general registers",
            "Dump contents of CPU general registers",
            NOCMD },
    { "goto",
            (fcmd_t) dr4kcpu_goto,
            DEFAULT,
            DEFAULT,
            "Go to address",
            "Go to address",
            REQ INT "addr/address" END },
    { "break",
            (fcmd_t) dr4kcpu_break,
            DEFAULT,
            DEFAULT,
            "Add code breakpoint",
            "Add code breakpoint",
            REQ INT "addr/address" END },
    { "bd",
            (fcmd_t) dr4kcpu_bd,
            DEFAULT,
            DEFAULT,
            "Dump code breakpoints",
            "Dump code breakpoints",
            NOCMD },
    { "br",
            (fcmd_t) dr4kcpu_br,
            DEFAULT,
            DEFAULT,
            "Remove code breakpoint",
            "Remove code breakpoint",
            REQ INT "addr/address" END },
    LAST_CMD
};

device_type_t dr4kcpu = {
    /* CPU is simulated deterministically */
    .nondet = false,

    /* Type name */
    .name = "dr4kcpu",

    /* Brief description*/
    .brief = "MIPS R4000 processor",

    /* Full description */
    .full = "MIPS R4000 processor restricted to 32 bits without FPU",

    /* Functions */
    .done = dr4kcpu_done,
    .step = dr4kcpu_step,

    /* Commands */
    .cmds = dr4kcpu_cmds
};
