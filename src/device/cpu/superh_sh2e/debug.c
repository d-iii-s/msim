/*
 * Copyright (c) X-Y Z
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
#include <stdio.h>
#include <string.h>

#include "../../../assert.h"
#include "../../../env.h"
#include "../../../main.h"
#include "../../../utils.h"
#include "cpu.h"
#include "debug.h"
#include "decode.h"
#include "memops.h"


/** Processor register names in various styles. */

static char const * const sh2e_general_regname_styles[__SH2E_REGNAME_STYLE_COUNT][SH2E_GENERAL_REG_COUNT] = {
    {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    },
    {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "sp"
    },
};

char const * const sh2e_control_reg_names[SH2E_CONTROL_REG_COUNT] = {
    "sr", "gbr", "vbr"
};

char const * const sh2e_system_reg_names[SH2E_SYSTEM_REG_COUNT] = {
    "mach", "macl", "pr", "pc"
};

static char const * const sh2e_fp_general_regname_styles[__SH2E_REGNAME_STYLE_COUNT][SH2E_FPU_GENERAL_REG_COUNT] = {
    {
        // fr0 functions as the index register for the FMAC instruction.
        "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7",
        "fr8", "fr9", "fr10", "fr11", "fr12", "fr13", "fr14", "fr15"
    },
    {
        // fr0 functions as the index register for the FMAC instruction.
        "fr0", "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7",
        "fr8", "fr9", "fr10", "fr11", "fr12", "fr13", "fr14", "fr15"
    },
};

char const * sh2e_fp_system_reg_names[SH2E_FPU_SYSTEM_REG_COUNT] = {
    "fpul", "fpscr",
};


char const * const * sh2e_cpu_general_reg_names;
char const * const * sh2e_fpu_general_reg_names;

static void
sh2e_set_regname_style(sh2e_regname_style_t style) {
    sh2e_cpu_general_reg_names = sh2e_general_regname_styles[style];
    sh2e_fpu_general_reg_names = sh2e_fp_general_regname_styles[style];
}


/**
 * @brief Initializes the processor debugging output.
 */
void
sh2e_debug_init(void) {
    sh2e_set_regname_style(SH2E_REGNAME_STYLE_ABI);
}


/**
 * @brief Change the names of the general purpose registers
 */
bool
sh2e_debug_set_regname_style(sh2e_regname_style_t style) {
    if (style < __SH2E_REGNAME_STYLE_COUNT) {
        sh2e_set_regname_style(style);
        return true;
    } else {
        error("Index out of range 0..%u", __SH2E_REGNAME_STYLE_COUNT - 1);
        return false;
    }
}


void
sh2e_cpu_dump_insn(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn
) {
    string_t s_cpu;
    string_init(&s_cpu);

    if (cpu != NULL) {
        string_printf(&s_cpu, "cpu%u", cpu->id);
    }

    string_t s_addr;
    string_init(&s_addr);
    string_printf(&s_addr, "%08" PRIX32, addr);

    string_t s_opcode;
    string_init(&s_opcode);
    string_printf(&s_opcode, "%04" PRIX16, insn.word);

    string_t s_mnemonics;
    string_init(&s_mnemonics);

    string_t s_comments;
    string_init(&s_comments);

    sh2e_insn_desc_t const * const desc = sh2e_insn_decode(insn);
    if (desc->disasm != NULL) {
        desc->disasm(desc, cpu, addr, insn, &s_mnemonics, &s_comments);
    } else {
        string_printf(&s_mnemonics, desc->assembly);
    }

    if (desc->abstract != NULL) {
        string_printf(&s_comments, "; %s", desc->abstract);
    }

    // Print the output.

    printf("%-7s", s_cpu.str);
    printf("%-10s", s_addr.str);
    printf("%-6s", s_opcode.str);
    printf("%-40s", s_mnemonics.str);
    printf("%s\n", s_comments.str);

    string_done(&s_comments);
    string_done(&s_mnemonics);
    string_done(&s_opcode);
    string_done(&s_addr);
    string_done(&s_cpu);
}


void
sh2e_cpu_dump_code(sh2e_cpu_t const * const restrict cpu, uint32_t const start_addr, unsigned int const count) {
    ASSERT(cpu != NULL);

    uint32_t const start_addr_aligned = ALIGN_UP(start_addr, sizeof(sh2e_insn_t));
    uint32_t const end_addr =  start_addr_aligned + count * sizeof(sh2e_insn_t);
    for (uint32_t addr = start_addr_aligned; addr < end_addr; addr += sizeof(sh2e_insn_t)) {
        sh2e_insn_t insn = { .word = sh2e_physmem_read16(cpu->id, addr, false) };
        sh2e_cpu_dump_insn(cpu, addr, insn);
    }
}


void
sh2e_cpu_dump_data(
    sh2e_cpu_t const * const restrict cpu,
    uint32_t const start_addr, unsigned int const count
) {
    ASSERT(cpu != NULL);

    uint32_t const start_addr_aligned = ALIGN_UP(start_addr, sizeof(uint32_t));
    unsigned int const step = sizeof(uint32_t);

    uint32_t addr = start_addr_aligned;
    for (unsigned int i = 0; i < count; i++) {
        if ((i % step) == 0) {
            printf("  %#08" PRIx32 "    ", addr);
        }

        uint32_t val = sh2e_physmem_read32(cpu->id, addr, false);
        printf("%08" PRIx32 " ", val);

        if ((i % step) == (step - 1)) {
            printf("\n");
        }

        addr += 4;
    }

    if ((count % step) != 0) {
        printf("\n");
    }
}


/**
 * @brief Dump the content of the general purpose registers to stdout.
 */
void
sh2e_cpu_dump_cpu_regs(sh2e_cpu_t const * const restrict cpu) {
    ASSERT(cpu != NULL);

    printf("processor %u\n", cpu->id);

    // General registers.
    for (unsigned int i = 0; i < SH2E_GENERAL_REG_COUNT; i += 4) {
        printf(
            " %5s: %8x %5s: %8x %5s: %8x %5s: %8x\n",
            sh2e_cpu_general_reg_names[i + 0], cpu->cpu_regs.general[i + 0],
            sh2e_cpu_general_reg_names[i + 1], cpu->cpu_regs.general[i + 1],
            sh2e_cpu_general_reg_names[i + 2], cpu->cpu_regs.general[i + 2],
            sh2e_cpu_general_reg_names[i + 3], cpu->cpu_regs.general[i + 3]
        );
    }

    // System registers.
    printf(
        " %5s: %8x %5s: %8x %5s: %8x %5s: %8x\n",
        sh2e_system_reg_names[3], cpu->cpu_regs.pc,
        sh2e_system_reg_names[2], cpu->cpu_regs.pr,
        sh2e_system_reg_names[0], cpu->cpu_regs.mach,
        sh2e_system_reg_names[1], cpu->cpu_regs.macl
    );

    // Control registers.
    printf(
        " %5s: %8x %5s: %8x %5s: %8x\n",
        sh2e_control_reg_names[0], cpu->cpu_regs.sr.value,
        sh2e_control_reg_names[1], cpu->cpu_regs.gbr,
        sh2e_control_reg_names[2], cpu->cpu_regs.vbr
    );
}


/**
 * @brief Dump the content of the general purpose registers to stdout.
 */
void
sh2e_cpu_dump_fpu_regs(sh2e_cpu_t const * const restrict cpu) {
    ASSERT(cpu != NULL);

    printf("processor %u\n", cpu->id);

    // General registers.
    for (unsigned int i = 0; i < SH2E_FPU_GENERAL_REG_COUNT; i += 4) {
        printf(
            " %5s: %-+12g %5s: %-+12g %5s: %-+12g %5s: %-+12g\n",
            sh2e_fpu_general_reg_names[i + 0], cpu->fpu_regs.general[i + 0],
            sh2e_fpu_general_reg_names[i + 1], cpu->fpu_regs.general[i + 1],
            sh2e_fpu_general_reg_names[i + 2], cpu->fpu_regs.general[i + 2],
            sh2e_fpu_general_reg_names[i + 3], cpu->fpu_regs.general[i + 3]
        );
    }

    // System registers.
    printf(
        " %5s: %-+12g %5s: %-8x     %5s: %-8x\n",
        sh2e_fp_system_reg_names[0], cpu->fpu_regs.fpul.fvalue,
        sh2e_fp_system_reg_names[0], cpu->fpu_regs.fpul.ivalue,
        sh2e_fp_system_reg_names[1], cpu->fpu_regs.fpscr.value
        // TODO Show FPSCR bits.
    );

    // TODO Control registers.
}


/**
 * @brief Dump instruction mnemonics.
 *
 * @param addr Physical address of the instruction.
 * @param insn The instruction to dump.
 */
void
sh2e_dump_insn_phys(ptr36_t const addr) {
    sh2e_insn_t insn = { .word = sh2e_physmem_read16(-1, addr, false) };
    sh2e_cpu_dump_insn(NULL, addr, insn);
}
