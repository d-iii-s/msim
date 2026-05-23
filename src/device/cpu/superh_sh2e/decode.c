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

#include <stddef.h>

#include "disasm.h"
#include "exec.h"
#include "insn.h"

static sh2e_insn_desc_t const illegal = {
    .assembly = "<ILLEGAL>",
    .exec = sh2e_insn_exec_illegal,
    .cycles = 1,
};

/****************************************************************************
 * SH-2E `0-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_z_format(sh2e_insn_z_t const insn)
{
    switch (insn.ic) {
    // Table A.25: 0 Format
    //   0b0000000000xxxxxx
    case sh2e_insn_z_ic_clrt: {
        static sh2e_insn_desc_t const clrt = {
            .assembly = "CLRT",
            .abstract = "0 → T",
            .exec = sh2e_insn_exec_clrt,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &clrt;
    }

    case sh2e_insn_z_ic_nop: {
        static sh2e_insn_desc_t const nop = {
            .assembly = "NOP",
            .abstract = "(no operation)",
            .exec = sh2e_insn_exec_nop,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &nop;
    }

    case sh2e_insn_z_ic_rts: {
        static sh2e_insn_desc_t const rts = {
            .assembly = "RTS",
            .abstract = "PR → PC (delayed)",
            .exec = sh2e_insn_exec_rts,
            .disasm = sh2e_insn_desc_dump_z_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &rts;
    }

    case sh2e_insn_z_ic_sett: {
        static sh2e_insn_desc_t const sett = {
            .assembly = "SETT",
            .abstract = "1 → T",
            .exec = sh2e_insn_exec_sett,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &sett;
    }

    case sh2e_insn_z_ic_div0u: {
        static sh2e_insn_desc_t const div0u = {
            .assembly = "DIV0U",
            .abstract = "0 → M/Q/T",
            .exec = sh2e_insn_exec_div0u,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &div0u;
    }

    case sh2e_insn_z_ic_sleep: {
        static sh2e_insn_desc_t const sleep = {
            .assembly = "SLEEP",
            .abstract = "(sleep)",
            .exec = sh2e_insn_exec_sleep,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 3,
        };
        return &sleep;
    }

    case sh2e_insn_z_ic_clrmac: {
        static sh2e_insn_desc_t const clrmac = {
            .assembly = "CLRMAC",
            .abstract = "0 → MACH/MACL",
            .exec = sh2e_insn_exec_clrmac,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &clrmac;
    }

    case sh2e_insn_z_ic_rte: {
        static sh2e_insn_desc_t const rte = {
            .assembly = "RTE",
            .abstract = "stack area → PC/SR (delayed)",
            .exec = sh2e_insn_exec_rte,
            .disasm = sh2e_insn_desc_dump_z_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 4,
        };
        return &rte;
    }
    default:
        return NULL;
    }
}

/****************************************************************************
 * SH-2E `n-format` decoding
 ****************************************************************************/

#define ic12(ic4, ic8) (((ic4) << 8) | (ic8))

static sh2e_insn_desc_t const *
sh2e_insn_decode_n_format(sh2e_insn_n_t const insn)
{
    uint16_t const ic = ic12(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.26: Direct Register
    case ic12(sh2e_insn_n_ic_h_cmppl, sh2e_insn_n_ic_l_cmppl): {
        static sh2e_insn_desc_t const cmppl = {
            .assembly = "CMP/PL Rn",
            .abstract = "Rn > 0 → T (signed)",
            .exec = sh2e_insn_exec_cmppl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &cmppl;
    }

    case ic12(sh2e_insn_n_ic_h_cmppz, sh2e_insn_n_ic_l_cmppz): {
        static sh2e_insn_desc_t const cmppz = {
            .assembly = "CMP/PZ Rn",
            .abstract = "Rn >= 0 → T (signed)",
            .exec = sh2e_insn_exec_cmppz,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &cmppz;
    }

    case ic12(sh2e_insn_n_ic_h_dt, sh2e_insn_n_ic_l_dt): {
        static sh2e_insn_desc_t const dt = {
            .assembly = "DT Rn",
            .abstract = "(Rn - 1) → Rn, (Rn == 0) → T",
            .exec = sh2e_insn_exec_dt,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &dt;
    }

    case ic12(sh2e_insn_n_ic_h_movt, sh2e_insn_n_ic_l_movt): {
        static sh2e_insn_desc_t const movt = {
            .assembly = "MOVT Rn",
            .abstract = "T → Rn",
            .exec = sh2e_insn_exec_movt,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &movt;
    }

    case ic12(sh2e_insn_n_ic_h_rotl, sh2e_insn_n_ic_l_rotl): {
        static sh2e_insn_desc_t const rotl = {
            .assembly = "ROTL Rn",
            .abstract = "T ← Rn ← MSB",
            .exec = sh2e_insn_exec_rotl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotl;
    }

    case ic12(sh2e_insn_n_ic_h_rotr, sh2e_insn_n_ic_l_rotr): {
        static sh2e_insn_desc_t const rotr = {
            .assembly = "ROTR Rn",
            .abstract = "LSB → Rn → T",
            .exec = sh2e_insn_exec_rotr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotr;
    }

    case ic12(sh2e_insn_n_ic_h_rotcl, sh2e_insn_n_ic_l_rotcl): {
        static sh2e_insn_desc_t const rotcl = {
            .assembly = "ROTCL Rn",
            .abstract = "T ← Rn ← T",
            .exec = sh2e_insn_exec_rotcl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotcl;
    }

    case ic12(sh2e_insn_n_ic_h_rotcr, sh2e_insn_n_ic_l_rotcr): {
        static sh2e_insn_desc_t const rotcr = {
            .assembly = "ROTCR Rn",
            .abstract = "T → Rn → T",
            .exec = sh2e_insn_exec_rotcr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotcr;
    }

    case ic12(sh2e_insn_n_ic_h_shal, sh2e_insn_n_ic_l_shal): {
        static sh2e_insn_desc_t const shal = {
            .assembly = "SHAL Rn",
            .abstract = "T ← Rn ← 0",
            .exec = sh2e_insn_exec_shal,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shal;
    }

    case ic12(sh2e_insn_n_ic_h_shar, sh2e_insn_n_ic_l_shar): {
        static sh2e_insn_desc_t const shar = {
            .assembly = "SHAR Rn",
            .abstract = "MSB → Rn → T",
            .exec = sh2e_insn_exec_shar,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shar;
    }

    case ic12(sh2e_insn_n_ic_h_shll, sh2e_insn_n_ic_l_shll): {
        static sh2e_insn_desc_t const shll = {
            .assembly = "SHLL Rn",
            .abstract = "T ← Rn ← 0",
            .exec = sh2e_insn_exec_shll,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll;
    }

    case ic12(sh2e_insn_n_ic_h_shlr, sh2e_insn_n_ic_l_shlr): {
        static sh2e_insn_desc_t const shlr = {
            .assembly = "SHLR Rn",
            .abstract = "0 → Rn → T",
            .exec = sh2e_insn_exec_shlr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr;
    }

    // SHLL scale:    0b00xx1000
    case ic12(sh2e_insn_n_ic_h_shll2, sh2e_insn_n_ic_l_shll2): {
        static sh2e_insn_desc_t const shll2 = {
            .assembly = "SHLL2 Rn",
            .abstract = "(Rn << 2) → Rn",
            .exec = sh2e_insn_exec_shll2,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll2;
    }

    case ic12(sh2e_insn_n_ic_h_shll8, sh2e_insn_n_ic_l_shll8): {
        static sh2e_insn_desc_t const shll8 = {
            .assembly = "SHLL8 Rn",
            .abstract = "(Rn << 8) → Rn",
            .exec = sh2e_insn_exec_shll8,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll8;
    }

    case ic12(sh2e_insn_n_ic_h_shll16, sh2e_insn_n_ic_l_shll16): {
        static sh2e_insn_desc_t const shll16 = {
            .assembly = "SHLL16 Rn",
            .abstract = "(Rn << 16) → Rn",
            .exec = sh2e_insn_exec_shll16,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll16;
    }

    // SHLR scale:    0b00xx1001
    case ic12(sh2e_insn_n_ic_h_shlr2, sh2e_insn_n_ic_l_shlr2): {
        static sh2e_insn_desc_t const shlr2 = {
            .assembly = "SHLR2 Rn",
            .abstract = "(Rn >> 2) → Rn",
            .exec = sh2e_insn_exec_shlr2,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr2;
    }

    case ic12(sh2e_insn_n_ic_h_shlr8, sh2e_insn_n_ic_l_shlr8): {
        static sh2e_insn_desc_t const shlr8 = {
            .assembly = "SHLR8 Rn",
            .abstract = "(Rn >> 8) → Rn",
            .exec = sh2e_insn_exec_shlr8,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr8;
    }

    case ic12(sh2e_insn_n_ic_h_shlr16, sh2e_insn_n_ic_l_shlr16): {
        static sh2e_insn_desc_t const shlr16 = {
            .assembly = "SHLR16 Rn",
            .abstract = "(Rn >> 16) → Rn",
            .exec = sh2e_insn_exec_shlr16,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr16;
    }

    // Table A.27: Direct Register (Store with Control and System Registers)
    // CPU ctl reg#:  0b00xx0010
    case ic12(sh2e_insn_n_ic_h_stc_sr, sh2e_insn_n_ic_l_stc_sr): {
        static sh2e_insn_desc_t const stc_sr = {
            .assembly = "STC SR, Rn",
            .abstract = "SR → Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_sr;
    }

    case ic12(sh2e_insn_n_ic_h_stc_gbr, sh2e_insn_n_ic_l_stc_gbr): {
        static sh2e_insn_desc_t const stc_gbr = {
            .assembly = "STC GBR, Rn",
            .abstract = "GBR → Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_gbr;
    }

    case ic12(sh2e_insn_n_ic_h_stc_vbr, sh2e_insn_n_ic_l_stc_vbr): {
        static sh2e_insn_desc_t const stc_vbr = {
            .assembly = "STC VBR, Rn",
            .abstract = "VBR → Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_vbr;
    }

    // FPU sys reg#:  0b01xx1010
    case ic12(sh2e_insn_n_ic_h_sts_fpul, sh2e_insn_n_ic_l_sts_fpul): {
        static sh2e_insn_desc_t const sts_fpul = {
            .assembly = "STS FPUL, Rn",
            .abstract = "FPUL → Rn",
            .exec = sh2e_insn_exec_sts_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &sts_fpul;
    }

    case ic12(sh2e_insn_n_ic_h_sts_fpscr, sh2e_insn_n_ic_l_sts_fpscr): {
        static sh2e_insn_desc_t const sts_fpscr = {
            .assembly = "STS FPSCR, Rn",
            .abstract = "FPSCR → Rn",
            .exec = sh2e_insn_exec_sts_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &sts_fpscr;
    }

    // CPU sys reg#:  0b00xx1010
    case ic12(sh2e_insn_n_ic_h_sts_mach, sh2e_insn_n_ic_l_sts_mach): {
        static sh2e_insn_desc_t const sts_mach = {
            .assembly = "STS MACH, Rn",
            .abstract = "MACH → Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_mach;
    }

    case ic12(sh2e_insn_n_ic_h_sts_macl, sh2e_insn_n_ic_l_sts_macl): {
        static sh2e_insn_desc_t const sts_macl = {
            .assembly = "STS MACL, Rn",
            .abstract = "MACL → Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_macl;
    }

    case ic12(sh2e_insn_n_ic_h_sts_pr, sh2e_insn_n_ic_l_sts_pr): {
        static sh2e_insn_desc_t const sts_pr = {
            .assembly = "STS PR, Rn",
            .abstract = "PR → Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_pr;
    }

    // Table A.28: Indirect Register
    case ic12(sh2e_insn_n_ic_h_tas, sh2e_insn_n_ic_l_tas): {
        static sh2e_insn_desc_t const tas = {
            .assembly = "TAS.B @Rn",
            .abstract = "([Rn] == 0) → T, 1 → MSb of [Rn]",
            .exec = sh2e_insn_exec_tas,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .bus_lock = true,
        };
        return &tas;
    }

    // Table A.29: Indirect Pre-Decrement Register
    // CPU ctl reg#:  0b00xx0011
    case ic12(sh2e_insn_n_ic_h_stcm_sr, sh2e_insn_n_ic_l_stcm_sr): {
        static sh2e_insn_desc_t const stcm_sr = {
            .assembly = "STC.L SR, @-Rn",
            .abstract = "Rn - 4 → Rn, SR → [Rn]",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_sr;
    }

    case ic12(sh2e_insn_n_ic_h_stcm_gbr, sh2e_insn_n_ic_l_stcm_gbr): {
        static sh2e_insn_desc_t const stcm_gbr = {
            .assembly = "STC.L GBR, @-Rn",
            .abstract = "Rn - 4 → Rn, GBR → [Rn]",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_gbr;
    }

    case ic12(sh2e_insn_n_ic_h_stcm_vbr, sh2e_insn_n_ic_l_stcm_vbr): {
        static sh2e_insn_desc_t const stcm_vbr = {
            .assembly = "STC.L VBR, @-Rn",
            .abstract = "Rn - 4 → Rn, VBR → [Rn]",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_vbr;
    }

    // FPU sys reg#:  0b01xx0010
    case ic12(sh2e_insn_n_ic_h_stsm_fpul, sh2e_insn_n_ic_l_stsm_fpul): {
        static sh2e_insn_desc_t const stsm_fpul = {
            .assembly = "STS.L FPUL, @-Rn",
            .abstract = "Rn - 4 → Rn, FPUL → [Rn]",
            .exec = sh2e_insn_exec_stsm_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &stsm_fpul;
    }

    case ic12(sh2e_insn_n_ic_h_stsm_fpscr, sh2e_insn_n_ic_l_stsm_fpscr): {
        static sh2e_insn_desc_t const stsm_fpscr = {
            .assembly = "STS.L FPSCR, @-Rn",
            .abstract = "Rn - 4 → Rn, FPSCR → [Rn]",
            .exec = sh2e_insn_exec_stsm_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &stsm_fpscr;
    }

    // CPU sys reg#:  0b00xx0010
    case ic12(sh2e_insn_n_ic_h_stsm_mach, sh2e_insn_n_ic_l_stsm_mach): {
        static sh2e_insn_desc_t const stsm_mach = {
            .assembly = "STS.L MACH, @-Rn",
            .abstract = "Rn - 4 → Rn, MACH → [Rn]",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_mach;
    }

    case ic12(sh2e_insn_n_ic_h_stsm_macl, sh2e_insn_n_ic_l_stsm_macl): {
        static sh2e_insn_desc_t const stsm_macl = {
            .assembly = "STS.L MACL, @-Rn",
            .abstract = "Rn - 4 → Rn, MACL → [Rn]",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_macl;
    }

    case ic12(sh2e_insn_n_ic_h_stsm_pr, sh2e_insn_n_ic_l_stsm_pr): { // STS.L PR, @-Rn
        static sh2e_insn_desc_t const stsm_pr = {
            .assembly = "STS.L PR, @-Rn",
            .abstract = "Rn - 4 → Rn, PR → [Rn]",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_pr;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_n_format_fpu(sh2e_insn_n_t const insn)
{
    switch (insn.ic_l) {
    // Table A.30: Floating-Point Instruction
    //   0bxxxx1101
    case sh2e_insn_n_ic_l_fsts: {
        static sh2e_insn_desc_t const fsts = {
            .assembly = "FSTS FPUL, FRn",
            .abstract = "FPUL → FRn",
            .exec = sh2e_insn_exec_fsts,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fsts;
    }

    case sh2e_insn_n_ic_l_float: {
        static sh2e_insn_desc_t const ffloat = {
            .assembly = "FLOAT FPUL, FRn",
            .abstract = "(float) FPUL → FRn",
            .exec = sh2e_insn_exec_float,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &ffloat;
    }

    case sh2e_insn_n_ic_l_fneg: {
        static sh2e_insn_desc_t const fneg = {
            .assembly = "FNEG FRn",
            .abstract = "-FRn → FRn",
            .exec = sh2e_insn_exec_fneg,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fneg;
    }

    case sh2e_insn_n_ic_l_fabs: {
        static sh2e_insn_desc_t const fabs = {
            .assembly = "FABS FRn",
            .abstract = "|FRn| → FRn",
            .exec = sh2e_insn_exec_fabs,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fabs;
    }

    case sh2e_insn_n_ic_l_fldi0: {
        static sh2e_insn_desc_t const fldi0 = {
            .assembly = "FLDI0 FRn",
            .abstract = "0.0 → FRn",
            .exec = sh2e_insn_exec_fldi0,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fldi0;
    }

    case sh2e_insn_n_ic_l_fldi1: {
        static sh2e_insn_desc_t const fldi1 = {
            .assembly = "FLDI1 FRn",
            .abstract = "1.0 → FRn",
            .exec = sh2e_insn_exec_fldi1,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fldi1;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `m-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_m_format(sh2e_insn_m_t const insn)
{
    uint16_t const ic = ic12(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.31: Direct Register (Load from Control and System Registers)
    // CPU ctl reg#:  0b00xx1110
    case ic12(sh2e_insn_m_ic_h_ldc_sr, sh2e_insn_m_ic_l_ldc_sr): {
        static sh2e_insn_desc_t const ldc_sr = {
            .assembly = "LDC Rm, SR",
            .abstract = "Rm → SR",
            .exec = sh2e_insn_exec_ldc_sr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &ldc_sr;
    }

    case ic12(sh2e_insn_m_ic_h_ldc_gbr, sh2e_insn_m_ic_l_ldc_gbr): {
        static sh2e_insn_desc_t const ldc_gbr = {
            .assembly = "LDC Rm, GBR",
            .abstract = "Rm → GBR",
            .exec = sh2e_insn_exec_ldc_gbr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &ldc_gbr;
    }

    case ic12(sh2e_insn_m_ic_h_ldc_vbr, sh2e_insn_m_ic_l_ldc_vbr): { // LDC Rm, VBR
        static sh2e_insn_desc_t const ldc_vbr = {
            .assembly = "LDC Rm, VBR",
            .abstract = "Rm → VBR",
            .exec = sh2e_insn_exec_ldc_vbr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &ldc_vbr;
    }

    // FPU sys reg#:  0b01xx1010
    case ic12(sh2e_insn_m_ic_h_lds_fpul, sh2e_insn_m_ic_l_lds_fpul): {
        static sh2e_insn_desc_t const lds_fpul = {
            .assembly = "LDS Rm, FPUL",
            .abstract = "Rm → FPUL",
            .exec = sh2e_insn_exec_lds_fpul,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &lds_fpul;
    }

    case ic12(sh2e_insn_m_ic_h_lds_fpscr, sh2e_insn_m_ic_l_lds_fpscr): {
        static sh2e_insn_desc_t const lds_fpscr = {
            .assembly = "LDS Rm, FPSCR",
            .abstract = "Rm → FPSCR",
            .exec = sh2e_insn_exec_lds_fpscr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &lds_fpscr;
    }

    // CPU sys reg#:  0b00xx1010
    case ic12(sh2e_insn_m_ic_h_lds_mach, sh2e_insn_m_ic_l_lds_mach): {
        static sh2e_insn_desc_t const lds_mach = {
            .assembly = "LDS Rm, MACH",
            .abstract = "Rm → MACH",
            .exec = sh2e_insn_exec_lds_mach,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &lds_mach;
    }

    case ic12(sh2e_insn_m_ic_h_lds_macl, sh2e_insn_m_ic_l_lds_macl): {
        static sh2e_insn_desc_t const lds_macl = {
            .assembly = "LDS Rm, MACL",
            .abstract = "Rm → MACL",
            .exec = sh2e_insn_exec_lds_macl,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &lds_macl;
    }

    case ic12(sh2e_insn_m_ic_h_lds_pr, sh2e_insn_m_ic_l_lds_pr): {
        static sh2e_insn_desc_t const lds_pr = {
            .assembly = "LDS Rm, PR",
            .abstract = "Rm → PR",
            .exec = sh2e_insn_exec_lds_pr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &lds_pr;
    }

    // Table A.32: Indirect Register
    case ic12(sh2e_insn_m_ic_h_jmp, sh2e_insn_m_ic_l_jmp): {
        static sh2e_insn_desc_t const jmp = {
            .assembly = "JMP @Rm",
            .abstract = "Rm → PC (delayed)",
            .exec = sh2e_insn_exec_jmp,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &jmp;
    }

    case ic12(sh2e_insn_m_ic_h_jsr, sh2e_insn_m_ic_l_jsr): {
        static sh2e_insn_desc_t const jsr = {
            .assembly = "JSR @Rm",
            .abstract = "PC + 4 → PR, Rm → PC (delayed)",
            .exec = sh2e_insn_exec_jsr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &jsr;
    }

    // Table A.33: Indirect Post-Increment Register
    // CPU ctl reg#:  0b00xx0111
    case ic12(sh2e_insn_m_ic_h_ldcl_sr, sh2e_insn_m_ic_l_ldcl_sr): {
        static sh2e_insn_desc_t const ldcl_sr = {
            .assembly = "LDC.L @Rm+, SR",
            .abstract = "[Rm] → SR, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldcl_sr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 3,
            .disable_interrupts = true,
        };
        return &ldcl_sr;
    }

    case ic12(sh2e_insn_m_ic_h_ldcl_gbr, sh2e_insn_m_ic_l_ldcl_gbr): {
        static sh2e_insn_desc_t const ldcl_gbr = {
            .assembly = "LDC.L @Rm+, GBR",
            .abstract = "[Rm] → GBR, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldcl_gbr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 3,
            .disable_interrupts = true,
        };
        return &ldcl_gbr;
    }

    case ic12(sh2e_insn_m_ic_h_ldcl_vbr, sh2e_insn_m_ic_l_ldcl_vbr): { // LDC.L @Rm+, VBR
        static sh2e_insn_desc_t const ldcl_vbr = {
            .assembly = "LDC.L @Rm+, VBR",
            .abstract = "[Rm] → GBR, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldcl_vbr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 3,
            .disable_interrupts = true,
        };
        return &ldcl_vbr;
    }

    // FPU sys reg#:  0b01xx0110
    case ic12(sh2e_insn_m_ic_h_ldsl_fpul, sh2e_insn_m_ic_l_ldsl_fpul): {
        static sh2e_insn_desc_t const ldsl_fpul = {
            .assembly = "LDS.L @Rm+, FPUL",
            .abstract = "[Rm] → FPUL, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldsl_fpul,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &ldsl_fpul;
    }

    case ic12(sh2e_insn_m_ic_h_ldsl_fpscr, sh2e_insn_m_ic_l_ldsl_fpscr): {
        static sh2e_insn_desc_t const ldsl_fpscr = {
            .assembly = "LDS.L @Rm+, FPSCR",
            .abstract = "[Rm] → FPSCR, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldsl_fpscr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &ldsl_fpscr;
    }

    // CPU sys reg#:  0b00xx0110
    case ic12(sh2e_insn_m_ic_h_ldsl_mach, sh2e_insn_m_ic_l_ldsl_mach): {
        static sh2e_insn_desc_t const ldsl_mach = {
            .assembly = "LDS.L @Rm+, MACH",
            .abstract = "[Rm] → MACH, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldsl_mach,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &ldsl_mach;
    }

    case ic12(sh2e_insn_m_ic_h_ldsl_macl, sh2e_insn_m_ic_l_ldsl_macl): {
        static sh2e_insn_desc_t const ldsl_macl = {
            .assembly = "LDS.L @Rm+, MACL",
            .abstract = "[Rm] → MACL, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldsl_macl,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &ldsl_macl;
    }

    case ic12(sh2e_insn_m_ic_h_ldsl_pr, sh2e_insn_m_ic_l_ldsl_pr): {
        static sh2e_insn_desc_t const ldsl_pr = {
            .assembly = "LDS.L @Rm+, PR",
            .abstract = "[Rm] → PR, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_ldsl_pr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 1,
        };
        return &ldsl_pr;
    }

    // Table A.34: PC Relative Addressing with Rn
    case ic12(sh2e_insn_m_ic_h_braf, sh2e_insn_m_ic_l_braf): {
        static sh2e_insn_desc_t const braf = {
            .assembly = "BRAF Rm",
            .abstract = "PC + 4 + Rm → PC (delayed)",
            .exec = sh2e_insn_exec_braf,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &braf;
    }

    case ic12(sh2e_insn_m_ic_h_bsrf, sh2e_insn_m_ic_l_bsrf): {
        static sh2e_insn_desc_t const bsrf = {
            .assembly = "BSRF Rm",
            .abstract = "PC + 4 → PR, PC + 4 + Rm → PC (delayed)",
            .exec = sh2e_insn_exec_bsrf,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &bsrf;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_m_format_fpu(sh2e_insn_m_t const insn)
{
    switch (insn.ic_l) {
    // Table A.35: Floating-Point Instructions
    case sh2e_insn_m_ic_l_flds: {
        static sh2e_insn_desc_t const flds = {
            .assembly = "FLDS FRm, FPUL",
            .abstract = "FRm → FPUL",
            .exec = sh2e_insn_exec_flds,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &flds;
    }

    case sh2e_insn_m_ic_l_ftrc: {
        static sh2e_insn_desc_t const ftrc = {
            .assembly = "FTRC FRm, FPUL",
            .abstract = "(long) FRm → FPUL",
            .exec = sh2e_insn_exec_ftrc,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &ftrc;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `nm-format` decoding
 ****************************************************************************/

#define ic8(ic8, ic4) (((ic8) << 4) | (ic4))

static sh2e_insn_desc_t const *
sh2e_insn_decode_nm_format(sh2e_insn_nm_t const insn)
{
    uint_fast8_t const ic = ic8(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.36: Direct Register
    case ic8(sh2e_insn_nm_ic_h_add, sh2e_insn_nm_ic_l_add): {
        static sh2e_insn_desc_t const add = {
            .assembly = "ADD Rm, Rn",
            .abstract = "Rn + Rm → Rn",
            .exec = sh2e_insn_exec_add,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &add;
    }

    case ic8(sh2e_insn_nm_ic_h_add, sh2e_insn_nm_ic_l_addc): {
        static sh2e_insn_desc_t const addc = {
            .assembly = "ADDC Rm, Rn",
            .abstract = "Rn + Rm + T → Rn (unsigned), carry → T",
            .exec = sh2e_insn_exec_addc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &addc;
    }

    case ic8(sh2e_insn_nm_ic_h_add, sh2e_insn_nm_ic_l_addv): {
        static sh2e_insn_desc_t const addv = {
            .assembly = "ADDV Rm, Rn",
            .abstract = "Rn + Rm → Rn (signed), overflow → T",
            .exec = sh2e_insn_exec_addv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &addv;
    }

    case ic8(sh2e_insn_nm_ic_h_and, sh2e_insn_nm_ic_l_and): {
        static sh2e_insn_desc_t const and = {
            .assembly = "AND Rm, Rn",
            .abstract = "Rn & Rm → Rn",
            .exec = sh2e_insn_exec_and,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &and;
    }

    case ic8(sh2e_insn_nm_ic_h_cmpeq, sh2e_insn_nm_ic_l_cmpeq): {
        static sh2e_insn_desc_t const cmpeq = {
            .assembly = "CMP/EQ Rm, Rn",
            .abstract = "Rn == Rm → T",
            .exec = sh2e_insn_exec_cmpeq,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpeq;
    }

    case ic8(sh2e_insn_nm_ic_h_cmphs, sh2e_insn_nm_ic_l_cmphs): {
        static sh2e_insn_desc_t const cmphs = {
            .assembly = "CMP/HS Rm, Rn",
            .abstract = "Rn >= Rm → T (unsigned)",
            .exec = sh2e_insn_exec_cmphs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmphs;
    }

    case ic8(sh2e_insn_nm_ic_h_cmpge, sh2e_insn_nm_ic_l_cmpge): {
        static sh2e_insn_desc_t const cmpge = {
            .assembly = "CMP/GE Rm, Rn",
            .abstract = "Rn >= Rm → T (signed)",
            .exec = sh2e_insn_exec_cmpge,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpge;
    }

    case ic8(sh2e_insn_nm_ic_h_cmphi, sh2e_insn_nm_ic_l_cmphi): {
        static sh2e_insn_desc_t const cmphi = {
            .assembly = "CMP/HI Rm, Rn",
            .abstract = "Rn > Rm → T (unsigned)",
            .exec = sh2e_insn_exec_cmphi,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmphi;
    }

    case ic8(sh2e_insn_nm_ic_h_cmpgt, sh2e_insn_nm_ic_l_cmpgt): {
        static sh2e_insn_desc_t const cmpgt = {
            .assembly = "CMP/GT Rm, Rn",
            .abstract = "Rn > Rm → T (signed)",
            .exec = sh2e_insn_exec_cmpgt,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpgt;
    }

    case ic8(sh2e_insn_nm_ic_h_cmpstr, sh2e_insn_nm_ic_l_cmpstr): {
        static sh2e_insn_desc_t const cmpstr = {
            .assembly = "CMP/STR Rm, Rn",
            .abstract = "any byte in Rn equals corresponding byte in Rm → T",
            .exec = sh2e_insn_exec_cmpstr,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpstr;
    }

    case ic8(sh2e_insn_nm_ic_h_div1, sh2e_insn_nm_ic_l_div1): {
        static sh2e_insn_desc_t const div1 = {
            .assembly = "DIV1 Rm, Rn",
            .abstract = "Rn ÷ Rm → T (1 step)",
            .exec = sh2e_insn_exec_div1,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &div1;
    }

    case ic8(sh2e_insn_nm_ic_h_div0s, sh2e_insn_nm_ic_l_div0s): {
        static sh2e_insn_desc_t const div0s = {
            .assembly = "DIV0S Rm, Rn",
            .abstract = "Rn{31} → Q, Rm{31} → M, Q ^ M → T",
            .exec = sh2e_insn_exec_div0s,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &div0s;
    }

    case ic8(sh2e_insn_nm_ic_h_dmulsl, sh2e_insn_nm_ic_l_dmulsl): {
        static sh2e_insn_desc_t const dmulsl = {
            .assembly = "DMULS.L Rm, Rn",
            .abstract = "Rn × Rm → MACH:MACL (signed)",
            .exec = sh2e_insn_exec_dmulsl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &dmulsl;
    }

    case ic8(sh2e_insn_nm_ic_h_dmulul, sh2e_insn_nm_ic_l_dmulul): {
        static sh2e_insn_desc_t const dmulul = {
            .assembly = "DMULU.L Rm, Rn",
            .abstract = "Rn × Rm → MACH:MACL (unsigned)",
            .exec = sh2e_insn_exec_dmulul,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &dmulul;
    }

    case ic8(sh2e_insn_nm_ic_h_extsb, sh2e_insn_nm_ic_l_extsb): {
        static sh2e_insn_desc_t const extsb = {
            .assembly = "EXTS.B Rm, Rn",
            .abstract = "SE(Rm{7:0}) → Rn",
            .exec = sh2e_insn_exec_extsb,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extsb;
    }

    case ic8(sh2e_insn_nm_ic_h_extsw, sh2e_insn_nm_ic_l_extsw): {
        static sh2e_insn_desc_t const extsw = {
            .assembly = "EXTS.W Rm, Rn",
            .abstract = "SE(Rm{15:0}) → Rn",
            .exec = sh2e_insn_exec_extsw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extsw;
    }

    case ic8(sh2e_insn_nm_ic_h_extub, sh2e_insn_nm_ic_l_extub): {
        static sh2e_insn_desc_t const extub = {
            .assembly = "EXTU.B Rm, Rn",
            .abstract = "ZE(Rm{7:0}) → Rn",
            .exec = sh2e_insn_exec_extub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extub;
    }

    case ic8(sh2e_insn_nm_ic_h_extuw, sh2e_insn_nm_ic_l_extuw): {
        static sh2e_insn_desc_t const extuw = {
            .assembly = "EXTU.W Rm, Rn",
            .abstract = "ZE(Rm{15:0}) → Rn",
            .exec = sh2e_insn_exec_extuw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extuw;
    }

    case ic8(sh2e_insn_nm_ic_h_mov, sh2e_insn_nm_ic_l_mov): {
        static sh2e_insn_desc_t const mov = {
            .assembly = "MOV Rm, Rn",
            .abstract = "Rm → Rn",
            .exec = sh2e_insn_exec_mov,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &mov;
    }

    case ic8(sh2e_insn_nm_ic_h_mull, sh2e_insn_nm_ic_l_mull): {
        static sh2e_insn_desc_t const mull = {
            .assembly = "MUL.L Rm, Rn",
            .abstract = "Rn × Rm → MACL",
            .exec = sh2e_insn_exec_mull,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &mull;
    }

    case ic8(sh2e_insn_nm_ic_h_mulsw, sh2e_insn_nm_ic_l_mulsw): {
        static sh2e_insn_desc_t const mulsw = {
            .assembly = "MULS.W Rm, Rn",
            .abstract = "Rn{15:0} × Rm{15:0} → MACL (signed)",
            .exec = sh2e_insn_exec_mulsw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1, // 1-3 cycles
        };
        return &mulsw;
    }

    case ic8(sh2e_insn_nm_ic_h_muluw, sh2e_insn_nm_ic_l_muluw): {
        static sh2e_insn_desc_t const muluw = {
            .assembly = "MULU.W Rm, Rn",
            .abstract = "Rn{15:0} × Rm{15:0} → MACL (unsigned)",
            .exec = sh2e_insn_exec_muluw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1, // 1-3 cycles
        };
        return &muluw;
    }

    case ic8(sh2e_insn_nm_ic_h_neg, sh2e_insn_nm_ic_l_neg): {
        static sh2e_insn_desc_t const neg = {
            .assembly = "NEG Rm, Rn",
            .abstract = "0 - Rm → Rn",
            .exec = sh2e_insn_exec_neg,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &neg;
    }

    case ic8(sh2e_insn_nm_ic_h_negc, sh2e_insn_nm_ic_l_negc): {
        static sh2e_insn_desc_t const negc = {
            .assembly = "NEGC Rm, Rn",
            .abstract = "0 - Rm - T → Rn, borrow → T",
            .exec = sh2e_insn_exec_negc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &negc;
    }

    case ic8(sh2e_insn_nm_ic_h_not, sh2e_insn_nm_ic_l_not): {
        static sh2e_insn_desc_t const not_insn = {
            .assembly = "NOT Rm, Rn",
            .abstract = "~Rm → Rn",
            .exec = sh2e_insn_exec_not,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &not_insn;
    }

    case ic8(sh2e_insn_nm_ic_h_or, sh2e_insn_nm_ic_l_or): {
        static sh2e_insn_desc_t const or = {
            .assembly = "OR Rm, Rn",
            .abstract = "Rn | Rm → Rn",
            .exec = sh2e_insn_exec_or,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &or ;
    }

    case ic8(sh2e_insn_nm_ic_h_sub, sh2e_insn_nm_ic_l_sub): {
        static sh2e_insn_desc_t const sub = {
            .assembly = "SUB Rm, Rn",
            .abstract = "Rn - Rm → Rn",
            .exec = sh2e_insn_exec_sub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &sub;
    }

    case ic8(sh2e_insn_nm_ic_h_subc, sh2e_insn_nm_ic_l_subc): {
        static sh2e_insn_desc_t const subc = {
            .assembly = "SUBC Rm, Rn",
            .abstract = "Rn - Rm - T → Rn (unsigned), borrow → T",
            .exec = sh2e_insn_exec_subc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &subc;
    }

    case ic8(sh2e_insn_nm_ic_h_subv, sh2e_insn_nm_ic_l_subv): {
        static sh2e_insn_desc_t const subv = {
            .assembly = "SUBV Rm, Rn",
            .abstract = "Rn - Rm → Rn (signed), underflow → T",
            .exec = sh2e_insn_exec_subv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &subv;
    }

    case ic8(sh2e_insn_nm_ic_h_swapb, sh2e_insn_nm_ic_l_swapb): {
        static sh2e_insn_desc_t const swapb = {
            .assembly = "SWAP.B Rm, Rn",
            .abstract = "Rm{31:16}:Rm{7:0}:Rm{15:8} → Rn",
            .exec = sh2e_insn_exec_swapb,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &swapb;
    }

    case ic8(sh2e_insn_nm_ic_h_swapw, sh2e_insn_nm_ic_l_swapw): {
        static sh2e_insn_desc_t const swapw = {
            .assembly = "SWAP.W Rm, Rn",
            .abstract = "Rm{15:0}:Rm{31:16} → Rn",
            .exec = sh2e_insn_exec_swapw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &swapw;
    }

    case ic8(sh2e_insn_nm_ic_h_tst, sh2e_insn_nm_ic_l_tst): {
        static sh2e_insn_desc_t const tst = {
            .assembly = "TST Rm, Rn",
            .abstract = "(Rn & Rm) == 0 → T",
            .exec = sh2e_insn_exec_tst,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &tst;
    }

    case ic8(sh2e_insn_nm_ic_h_xor, sh2e_insn_nm_ic_l_xor): {
        static sh2e_insn_desc_t const xor = {
            .assembly = "XOR Rm, Rn",
            .abstract = "Rn ^ Rm → Rn",
            .exec = sh2e_insn_exec_xor,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &xor;
    }

    case ic8(sh2e_insn_nm_ic_h_xtrct, sh2e_insn_nm_ic_l_xtrct): {
        static sh2e_insn_desc_t const xtrct = {
            .assembly = "XTRCT Rm, Rn",
            .abstract = "Rm{15:0}:Rn{31:16} → Rn",
            .exec = sh2e_insn_exec_xtrct,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &xtrct;
    }

    // Table A.37: Indirect Register
    case ic8(sh2e_insn_nm_ic_h_movbs, sh2e_insn_nm_ic_l_movbs): {
        static sh2e_insn_desc_t const movbs = {
            .assembly = "MOV.B Rm, @Rn",
            .abstract = "Rm{7:0} → [Rn]",
            .exec = sh2e_insn_exec_movbs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbs;
    }

    case ic8(sh2e_insn_nm_ic_h_movws, sh2e_insn_nm_ic_l_movws): {
        static sh2e_insn_desc_t const movws = {
            .assembly = "MOV.W Rm, @Rn",
            .abstract = "Rm{15:0} → [Rn]",
            .exec = sh2e_insn_exec_movws,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movws;
    }

    case ic8(sh2e_insn_nm_ic_h_movls, sh2e_insn_nm_ic_l_movls): {
        static sh2e_insn_desc_t const movls = {
            .assembly = "MOV.L Rm, @Rn",
            .abstract = "Rm → [Rn]",
            .exec = sh2e_insn_exec_movls,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movls;
    }

    case ic8(sh2e_insn_nm_ic_h_movbl, sh2e_insn_nm_ic_l_movbl): {
        static sh2e_insn_desc_t const movbl = {
            .assembly = "MOV.B @Rm, Rn",
            .abstract = "SE([Rm]{7:0}) → Rn",
            .exec = sh2e_insn_exec_movbl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbl;
    }

    case ic8(sh2e_insn_nm_ic_h_movwl, sh2e_insn_nm_ic_l_movwl): {
        static sh2e_insn_desc_t const movwl = {
            .assembly = "MOV.W @Rm, Rn",
            .abstract = "SE([Rm]{15:0}) → Rn",
            .exec = sh2e_insn_exec_movwl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwl;
    }

    case ic8(sh2e_insn_nm_ic_h_movll, sh2e_insn_nm_ic_l_movll): {
        static sh2e_insn_desc_t const movll = {
            .assembly = "MOV.L @Rm, Rn",
            .abstract = "[Rm] → Rn",
            .exec = sh2e_insn_exec_movll,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movll;
    }

    // Table A.38: Indirect Post-Increment Register (Multiply/Accumulate Operation)
    case ic8(sh2e_insn_nm_ic_h_macl, sh2e_insn_nm_ic_l_macl): {
        static sh2e_insn_desc_t const macl = {
            .assembly = "MAC.L @Rm+, @Rn+",
            .abstract = "[Rn] × [Rm] + MAC → MAC (signed), Rn + 4 → Rn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_macl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 3, // 2 to 4
        };
        return &macl;
    }

    case ic8(sh2e_insn_nm_ic_h_macw, sh2e_insn_nm_ic_l_macw): {
        static sh2e_insn_desc_t const macw = {
            .assembly = "MAC.W @Rm+, @Rn+",
            .abstract = "[Rn]{15:0} × [Rm]{15:0} + MAC → MAC (signed), Rn + 2 → Rn, Rm + 2 → Rm",
            .exec = sh2e_insn_exec_macw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 3, // 2 to 3
        };
        return &macw;
    }

    // Table A.39: Indirect Post-Increment Register
    case ic8(sh2e_insn_nm_ic_h_movbp, sh2e_insn_nm_ic_l_movbp): {
        static sh2e_insn_desc_t const movbp = {
            .assembly = "MOV.B @Rm+, Rn",
            .abstract = "SE([Rm]{7:0}) → Rn, Rm + 1 → Rm",
            .exec = sh2e_insn_exec_movbp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbp;
    }

    case ic8(sh2e_insn_nm_ic_h_movwp, sh2e_insn_nm_ic_l_movwp): {
        static sh2e_insn_desc_t const movwp = {
            .assembly = "MOV.W @Rm+, Rn",
            .abstract = "SE([Rm]{15:0}) → Rn, Rm + 2 → Rm",
            .exec = sh2e_insn_exec_movwp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwp;
    }

    case ic8(sh2e_insn_nm_ic_h_movlp, sh2e_insn_nm_ic_l_movlp): {
        static sh2e_insn_desc_t const movlp = {
            .assembly = "MOV.L @Rm+, Rn",
            .abstract = "[Rm] → Rn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_movlp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movlp;
    }

    // Table A.40: Indirect Pre-Decrement Register
    case ic8(sh2e_insn_nm_ic_h_movbm, sh2e_insn_nm_ic_l_movbm): {
        static sh2e_insn_desc_t const movbm = {
            .assembly = "MOV.B Rm, @-Rn",
            .abstract = "Rn - 1 → Rn, Rm{7:0} → [Rn]",
            .exec = sh2e_insn_exec_movbm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbm;
    }

    case ic8(sh2e_insn_nm_ic_h_movwm, sh2e_insn_nm_ic_l_movwm): {
        static sh2e_insn_desc_t const movwm = {
            .assembly = "MOV.W Rm, @-Rn",
            .abstract = "Rn - 2 → Rn, Rm{15:0} → [Rn]",
            .exec = sh2e_insn_exec_movwm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwm;
    }

    case ic8(sh2e_insn_nm_ic_h_movlm, sh2e_insn_nm_ic_l_movlm): {
        static sh2e_insn_desc_t const movlm = {
            .assembly = "MOV.L Rm, @-Rn",
            .abstract = "Rn - 4 → Rn, Rm → [Rn]",
            .exec = sh2e_insn_exec_movlm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movlm;
    }

    // Table A.41: Indirect Indexed Register
    case ic8(sh2e_insn_nm_ic_h_movbs0, sh2e_insn_nm_ic_l_movbs0): {
        static sh2e_insn_desc_t const movbs0 = {
            .assembly = "MOV.B Rm, @(R0, Rn)",
            .abstract = "Rm{7:0} → [R0 + Rn]",
            .exec = sh2e_insn_exec_movbs0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbs0;
    }

    case ic8(sh2e_insn_nm_ic_h_movws0, sh2e_insn_nm_ic_l_movws0): {
        static sh2e_insn_desc_t const movws0 = {
            .assembly = "MOV.W Rm, @(R0, Rn)",
            .abstract = "Rm{15:0} → [R0 + Rn]",
            .exec = sh2e_insn_exec_movws0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movws0;
    }

    case ic8(sh2e_insn_nm_ic_h_movls0, sh2e_insn_nm_ic_l_movls0): {
        static sh2e_insn_desc_t const movls0 = {
            .assembly = "MOV.L Rm, @(R0, Rn)",
            .abstract = "Rm → [R0 + Rn]",
            .exec = sh2e_insn_exec_movls0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movls0;
    }

    case ic8(sh2e_insn_nm_ic_h_movbl0, sh2e_insn_nm_ic_l_movbl0): {
        static sh2e_insn_desc_t const movbl0 = {
            .assembly = "MOV.B @(R0, Rm), Rn",
            .abstract = "SE([R0 + Rm]{7:0}) → Rn",
            .exec = sh2e_insn_exec_movbl0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbl0;
    }

    case ic8(sh2e_insn_nm_ic_h_movwl0, sh2e_insn_nm_ic_l_movwl0): {
        static sh2e_insn_desc_t const movwl0 = {
            .assembly = "MOV.W @(R0, Rm), Rn",
            .abstract = "SE([R0 + Rm]{15:0}) → Rn",
            .exec = sh2e_insn_exec_movwl0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwl0;
    }

    case ic8(sh2e_insn_nm_ic_h_movll0, sh2e_insn_nm_ic_l_movll0): {
        static sh2e_insn_desc_t const movll0 = {
            .assembly = "MOV.L @(R0, Rm), Rn",
            .abstract = "[R0 + Rm] → Rn",
            .exec = sh2e_insn_exec_movll0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movll0;
    }

    default: {
        break;
    }
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_nm_format_fpu(sh2e_insn_nm_t const insn)
{
    switch (insn.ic_l) {
    // Table A.42: Floating Point Instructions
    case sh2e_insn_nm_ic_l_fadd: {
        static sh2e_insn_desc_t const fadd = {
            .assembly = "FADD FRm, FRn",
            .abstract = "FRn + FRm → FRn",
            .exec = sh2e_insn_exec_fadd,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fadd;
    }

    case sh2e_insn_nm_ic_l_fcmpeq: {
        static sh2e_insn_desc_t const fcmpeq = {
            .assembly = "FCMP/EQ FRm, FRn",
            .abstract = "FRn == FRm → T",
            .exec = sh2e_insn_exec_fcmpeq,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fcmpeq;
    }

    case sh2e_insn_nm_ic_l_fcmpgt: {
        static sh2e_insn_desc_t const fcmpgt = {
            .assembly = "FCMP/GT FRm, FRn",
            .abstract = "FRn > FRm → T",
            .exec = sh2e_insn_exec_fcmpgt,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fcmpgt;
    }

    case sh2e_insn_nm_ic_l_fdiv: {
        static sh2e_insn_desc_t const fdiv = {
            .assembly = "FDIV FRm, FRn",
            .abstract = "FRn ÷ FRm → FRn",
            .exec = sh2e_insn_exec_fdiv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 13,
        };
        return &fdiv;
    }

    case sh2e_insn_nm_ic_l_fmac: {
        static sh2e_insn_desc_t const fmac = {
            .assembly = "FMAC FR0, FRm, FRn",
            .abstract = "FR0 × FRm + FRn → FRn",
            .exec = sh2e_insn_exec_fmac,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmac;
    }

    case sh2e_insn_nm_ic_l_fmov: {
        static sh2e_insn_desc_t const fmov = {
            .assembly = "FMOV FRm, FRn",
            .abstract = "FRm → FRn",
            .exec = sh2e_insn_exec_fmov,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmov;
    }

    case sh2e_insn_nm_ic_l_fmovli: {
        static sh2e_insn_desc_t const fmovli = {
            .assembly = "FMOV.S @(R0, Rm), FRn",
            .abstract = "[R0 + Rm] → FRn",
            .exec = sh2e_insn_exec_fmovli,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovli;
    }

    case sh2e_insn_nm_ic_l_fmovlr: {
        static sh2e_insn_desc_t const fmovlr = {
            .assembly = "FMOV.S @Rm+, FRn",
            .abstract = "[Rm] → FRn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_fmovlr,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovlr;
    }

    case sh2e_insn_nm_ic_l_fmovl: {
        static sh2e_insn_desc_t const fmovl = {
            .assembly = "FMOV.S @Rm, FRn",
            .abstract = "[Rm] → FRn",
            .exec = sh2e_insn_exec_fmovl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovl;
    }

    case sh2e_insn_nm_ic_l_fmovsi: {
        static sh2e_insn_desc_t const fmovsi = {
            .assembly = "FMOV.S FRm, @(R0, Rn)",
            .abstract = "FRm → [R0 + Rn]",
            .exec = sh2e_insn_exec_fmovsi,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovsi;
    }

    case sh2e_insn_nm_ic_l_fmovss: {
        static sh2e_insn_desc_t const fmovss = {
            .assembly = "FMOV.S FRm, @-Rn",
            .abstract = "Rn - 4 → Rn, FRm → [Rn]",
            .exec = sh2e_insn_exec_fmovss,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovss;
    }

    case sh2e_insn_nm_ic_l_fmovs: {
        static sh2e_insn_desc_t const fmovs = {
            .assembly = "FMOV.S FRm, @Rn",
            .abstract = "FRm → [Rn]",
            .exec = sh2e_insn_exec_fmovs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovs;
    }

    case sh2e_insn_nm_ic_l_fmul: {
        static sh2e_insn_desc_t const fmul = {
            .assembly = "FMUL FRm, FRn",
            .abstract = "FRn × FRm → FRn",
            .exec = sh2e_insn_exec_fmul,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmul;
    }

    case sh2e_insn_nm_ic_l_fsub: {
        static sh2e_insn_desc_t const fsub = {
            .assembly = "FSUB FRm, FRn",
            .abstract = "FRn - FRm → FRn",
            .exec = sh2e_insn_exec_fsub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fsub;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `md-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_md_format(sh2e_insn_md_t const insn)
{
    switch (insn.ic) {
    // Table A.43: Indirect register addressing with displacement
    case sh2e_insn_md_ic_movbl4: {
        static sh2e_insn_desc_t const movbl4 = {
            .assembly = "MOV.B @(disp, Rm), R0",
            .abstract = "SE([Rm + ZE(disp)]) → R0",
            .exec = sh2e_insn_exec_movbl4,
            .disasm = sh2e_insn_desc_dump_md_format_movb4,
            .cycles = 1,

        };
        return &movbl4;
    }

    case sh2e_insn_md_ic_movwl4: {
        static sh2e_insn_desc_t const movwl4 = {
            .assembly = "MOV.W @(disp, Rm), R0",
            .abstract = "SE([Rm + ZE(disp) × 2]) → R0",
            .exec = sh2e_insn_exec_movwl4,
            .disasm = sh2e_insn_desc_dump_md_format_movw4,
            .cycles = 1,

        };
        return &movwl4;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `nd4-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nd4_format(sh2e_insn_nd4_t const insn)
{
    switch (insn.ic) {
    // Table A.44: Indirect register addressing with displacement
    case sh2e_insn_nd4_ic_movbs4: {
        static sh2e_insn_desc_t const movbs4 = {
            .assembly = "MOV.B R0, @(disp, Rn)",
            .abstract = "R0{7:0} → [Rn + ZE(disp)]",
            .exec = sh2e_insn_exec_movbs4,
            .disasm = sh2e_insn_desc_dump_nd4_format_movb4,
            .cycles = 1,

        };
        return &movbs4;
    }

    case sh2e_insn_nd4_ic_movws4: {
        static sh2e_insn_desc_t const movws4 = {
            .assembly = "MOV.W R0, @(disp, Rn)",
            .abstract = "R0{15:0} → [Rn + ZE(disp) × 2]",
            .exec = sh2e_insn_exec_movws4,
            .disasm = sh2e_insn_desc_dump_nd4_format_movw4,
            .cycles = 1,

        };
        return &movws4;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `nmd-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nmd_format(sh2e_insn_nmd_t const insn)
{
    switch (insn.ic) {
    // Table A.45: Indirect register addressing with displacement
    case sh2e_insn_nmd_ic_movls4: {
        static sh2e_insn_desc_t const movls4 = {
            .assembly = "MOV.L Rm, @(disp, Rn)",
            .abstract = "Rm → [Rn + ZE(disp) × 4]",
            .exec = sh2e_insn_exec_movls4,
            .disasm = sh2e_insn_desc_dump_nmd_format,
            .cycles = 1,

        };
        return &movls4;
    }

    case sh2e_insn_nmd_ic_movll4: {
        static sh2e_insn_desc_t const movll4 = {
            .assembly = "MOV.L @(disp, Rm), Rn",
            .abstract = "[Rm + ZE(disp) × 4] → Rn",
            .exec = sh2e_insn_exec_movll4,
            .disasm = sh2e_insn_desc_dump_nmd_format,
            .cycles = 1,
        };
        return &movll4;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `d-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_d_format(sh2e_insn_d_t const insn)
{
    switch (insn.ic) {
    // Table A.46: Indirect GBR addressing with Displacement
    case sh2e_insn_d_ic_movblg: {
        static sh2e_insn_desc_t const movblg = {
            .assembly = "MOV.B @(disp, GBR), R0",
            .abstract = "SE([GBR + ZE(disp)]) → R0",
            .exec = sh2e_insn_exec_movblg,
            .disasm = sh2e_insn_desc_dump_d_format_movbg,
            .cycles = 1,
        };
        return &movblg;
    }

    case sh2e_insn_d_ic_movwlg: {
        static sh2e_insn_desc_t const movwlg = {
            .assembly = "MOV.W @(disp, GBR), R0",
            .abstract = "SE([GBR + ZE(disp) × 2]) → R0",
            .exec = sh2e_insn_exec_movwlg,
            .disasm = sh2e_insn_desc_dump_d_format_movwg,
            .cycles = 1,
        };
        return &movwlg;
    }

    case sh2e_insn_d_ic_movllg: {
        static sh2e_insn_desc_t const movllg = {
            .assembly = "MOV.L @(disp, GBR), R0",
            .abstract = "[GBR + ZE(disp) × 4] → R0",
            .exec = sh2e_insn_exec_movllg,
            .disasm = sh2e_insn_desc_dump_d_format_movlg,
            .cycles = 1,
        };
        return &movllg;
    }

    case sh2e_insn_d_ic_movbsg: {
        static sh2e_insn_desc_t const movbsg = {
            .assembly = "MOV.B R0, @(disp, GBR)",
            .abstract = "R0{7:0} → [GBR + ZE(disp)]",
            .exec = sh2e_insn_exec_movbsg,
            .disasm = sh2e_insn_desc_dump_d_format_movbg,
            .cycles = 1,
        };
        return &movbsg;
    }

    case sh2e_insn_d_ic_movwsg: {
        static sh2e_insn_desc_t const movwsg = {
            .assembly = "MOV.W R0, @(disp, GBR)",
            .abstract = "R0{15:0} → [GBR + ZE(disp) × 2]",
            .exec = sh2e_insn_exec_movwsg,
            .disasm = sh2e_insn_desc_dump_d_format_movwg,
            .cycles = 1,
        };
        return &movwsg;
    }

    case sh2e_insn_d_ic_movlsg: {
        static sh2e_insn_desc_t const movlsg = {
            .assembly = "MOV.L R0, @(disp, GBR)",
            .abstract = "R0 → [GBR + ZE(disp) × 4]",
            .exec = sh2e_insn_exec_movlsg,
            .disasm = sh2e_insn_desc_dump_d_format_movlg,
            .cycles = 1,
        };
        return &movlsg;
    }

    // Table A.47: PC Relative with Displacement
    case sh2e_insn_d_ic_mova: { //
        static sh2e_insn_desc_t const mova = {
            .assembly = "MOVA @(disp, PC), R0",
            .abstract = "[PC + ZE(disp) × 4] → R0",
            .exec = sh2e_insn_exec_mova,
            .disasm = sh2e_insn_desc_dump_d_format_mova,
            .cycles = 1,
        };
        return &mova;
    }

    // Table A.48: PC Relative addressing
    case sh2e_insn_d_ic_bf: {
        static sh2e_insn_desc_t const bf = {
            .assembly = "BF disp",
            .abstract = "if T = 0, PC + SE(disp) × 2 → PC",
            .exec = sh2e_insn_exec_bf,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .cycles = 1,
            .branch_cycles = 2,
        };
        return &bf;
    }

    case sh2e_insn_d_ic_bfs: {
        static sh2e_insn_desc_t const bfs = {
            .assembly = "BF/S disp",
            .abstract = "if T = 0, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bfs,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 1,
            .branch_cycles = 1,
        };
        return &bfs;
    }

    case sh2e_insn_d_ic_bt: {
        static sh2e_insn_desc_t const bt = {
            .assembly = "BT disp",
            .abstract = "if T = 1, PC + 4 + SE(disp) × 2 → PC",
            .exec = sh2e_insn_exec_bt,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .cycles = 1,
            .branch_cycles = 2,
        };
        return &bt;
    }

    case sh2e_insn_d_ic_bts: {
        static sh2e_insn_desc_t const bts = {
            .assembly = "BT/S disp",
            .abstract = "if T = 1, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bts,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 1,
            .branch_cycles = 1,
        };
        return &bts;
    }
    default: {
        return NULL;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `d12-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_d12_format(sh2e_insn_d12_t const insn)
{
    switch (insn.ic) {
    // Table A.49: PC relative addressing
    case sh2e_insn_d12_ic_bra: {
        static sh2e_insn_desc_t const bra = {
            .assembly = "BRA disp",
            .abstract = "PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bra,
            .disasm = sh2e_insn_desc_dump_d12_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &bra;
    }

    case sh2e_insn_d12_ic_bsr: {
        static sh2e_insn_desc_t const bsr = {
            .assembly = "BSR disp",
            .abstract = "PC + 4 → PR, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bsr,
            .disasm = sh2e_insn_desc_dump_d12_format,
            .disable_interrupts = true,
            .disable_address_errors = true,
            .cycles = 2,
        };
        return &bsr;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `nd8-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nd8_format(sh2e_insn_nd8_t const insn)
{
    switch (insn.ic) {
    // Table A.50: PC relative addressing with displacement
    case sh2e_insn_nd8_ic_movwi: {
        static sh2e_insn_desc_t const movwi = {
            .assembly = "MOV.W @(disp, PC), Rn",
            .abstract = "SE([PC + ZE(disp) × 2]) → Rn",
            .exec = sh2e_insn_exec_movwi,
            .disasm = sh2e_insn_desc_dump_nd8_format_movwi,
            .cycles = 1,
        };
        return &movwi;
    }

    case sh2e_insn_nd8_ic_movli: {
        static sh2e_insn_desc_t const movli = {
            .assembly = "MOV.L @(disp, PC), Rn",
            .abstract = "[PC + ZE(disp) × 4] → Rn",
            .exec = sh2e_insn_exec_movli,
            .disasm = sh2e_insn_desc_dump_nd8_format_movli,
            .cycles = 1,
        };
        return &movli;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `i-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_i_format(sh2e_insn_i_t const insn)
{
    switch (insn.ic) {
    // Table A.51: Indirect Indexed GBR
    case sh2e_insn_i_ic_tstm: {
        static sh2e_insn_desc_t const tstm = {
            .assembly = "TST.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] & #imm == 0 → T",
            .exec = sh2e_insn_exec_tstm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &tstm;
    }

    case sh2e_insn_i_ic_andm: {
        static sh2e_insn_desc_t const andm = {
            .assembly = "AND.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] & #imm → [GBR + R0]",
            .exec = sh2e_insn_exec_andm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &andm;
    }

    case sh2e_insn_i_ic_xorm: {
        static sh2e_insn_desc_t const xorm = {
            .assembly = "XOR.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] ^ imm → [GBR + R0]",
            .exec = sh2e_insn_exec_xorm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &xorm;
    }

    case sh2e_insn_i_ic_orm: {
        static sh2e_insn_desc_t const orm = {
            .assembly = "OR.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] | imm → [GBR + R0]",
            .exec = sh2e_insn_exec_orm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &orm;
    }

    // Table A.52: Immediate (Arithmetic Logical Operation with Direct Register)
    case sh2e_insn_i_ic_cmpim: {
        static sh2e_insn_desc_t const cmpim = {
            .assembly = "CMP/EQ #imm, R0",
            .abstract = "R0 == SE(#imm) → T",
            .exec = sh2e_insn_exec_cmpim,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &cmpim;
    }

    case sh2e_insn_i_ic_tsti: {
        static sh2e_insn_desc_t const tsti = {
            .assembly = "TST #imm, R0",
            .abstract = "R0 & ZE(#imm) == 0 → T",
            .exec = sh2e_insn_exec_tsti,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &tsti;
    }

    case sh2e_insn_i_ic_andi: {
        static sh2e_insn_desc_t const andi = {
            .assembly = "AND #imm, R0",
            .abstract = "R0 & ZE(#imm) → R0",
            .exec = sh2e_insn_exec_andi,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &andi;
    }

    case sh2e_insn_i_ic_xori: {
        static sh2e_insn_desc_t const xori = {
            .assembly = "XOR #imm, R0",
            .abstract = "R0 ^ ZE(#imm) → R0",
            .exec = sh2e_insn_exec_xori,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &xori;
    }

    case sh2e_insn_i_ic_ori: {
        static sh2e_insn_desc_t const ori = {
            .assembly = "OR #imm, R0",
            .abstract = "R0 | ZE(#imm) → R0",
            .exec = sh2e_insn_exec_ori,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &ori;
    }

    // Table A.53: Immediate (Specify Exception Processing Vector)
    case sh2e_insn_i_ic_trapa: {
        static sh2e_insn_desc_t const trapa = {
            .assembly = "TRAPA #imm",
            // NOTE: works like this only on SH1* and SH2*
            .abstract = "PC/SR -> stack area, (imm × 4 + VBR) → PC",
            .exec = sh2e_insn_exec_trapa,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 8,
        };
        return &trapa;
    }

    default: {
        return NULL;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `ni-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_ni_format(sh2e_insn_ni_t const insn)
{
    // Table A.54: Immediate addressing (direct register arithmetic operations and data transfers)
    switch (insn.ic) {
    case sh2e_insn_ni_ic_addi: {
        static sh2e_insn_desc_t const addi = {
            .assembly = "ADD #imm, Rn",
            .abstract = "Rn + SE(#imm) → Rn",
            .exec = sh2e_insn_exec_addi,
            .disasm = sh2e_insn_desc_dump_ni_format,
            .cycles = 1,
        };
        return &addi;
    }

    case sh2e_insn_ni_ic_movi: {
        static sh2e_insn_desc_t const movi = {
            .assembly = "MOV #imm, Rn",
            .abstract = "SE(#imm) → Rn",
            .exec = sh2e_insn_exec_movi,
            .disasm = sh2e_insn_desc_dump_ni_format,
            .cycles = 1,
        };
        return &movi;
    }
    }

    return NULL;
}

/****************************************************************************
 * SH-2E `MSIM`-format decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_msim_format(sh2e_insn_z_t const insn)
{
    switch (insn.ic) {

    // MSIM specific instruction to halt the machine
    // .word 0x8200
    case sh2e_insn_z_ic_halt: {
        static sh2e_insn_desc_t const halt = {
            .assembly = "HALT",
            .abstract = "(halt machine)",
            .exec = sh2e_insn_exec_halt,
            .disasm = sh2e_insn_desc_dump_msim_format,
            .cycles = 1,
        };
        return machine_specific_instructions ? &halt : &illegal;
    }

    // .word 0x8201
    case sh2e_insn_z_ic_cpu_reg_dump: {
        static sh2e_insn_desc_t const cpu_reg_dump = {
            .assembly = "CPU REG DUMP",
            .abstract = "(cpu registers dump)",
            .exec = sh2e_insn_exec_cpu_reg_dump,
            .disasm = sh2e_insn_desc_dump_msim_format,
            .cycles = 1,
        };
        return machine_specific_instructions ? &cpu_reg_dump : &illegal;
    }

    // .word 0x8202
    case sh2e_insn_z_ic_fpu_reg_dump: {
        static sh2e_insn_desc_t const fpu_reg_dump = {
            .assembly = "FPU REG DUMP",
            .abstract = "(fpu registers dump)",
            .exec = sh2e_insn_exec_fpu_reg_dump,
            .disasm = sh2e_insn_desc_dump_msim_format,
            .cycles = 1,
        };
        return machine_specific_instructions ? &fpu_reg_dump : &illegal;
    }
    default: {
        return NULL;
    }
    }
}

/****************************************************************************
 * SH-2E instruction decoding
 ****************************************************************************/

sh2e_insn_desc_t const *
sh2e_insn_decode(sh2e_insn_t const insn)
{
    sh2e_insn_desc_t const *desc = NULL;

    // Decode using the top 4 bits.
    // See Table A.56: Operation Code Map.
    switch (insn.word >> 12) {
    case 0b0000: // z_format, n_format, m_format, nm_format
        desc = sh2e_insn_decode_z_format(insn.z_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_n_format(insn.n_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_m_format(insn.m_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_nm_format(insn.nm_form);
        break;

    case 0b0001: // nmd_format
    case 0b0101: // nmd_format
        desc = sh2e_insn_decode_nmd_format(insn.nmd_form);
        break;

    case 0b0010: // nm_format
    case 0b0011: // nm_format
    case 0b0110: // nm_format
        desc = sh2e_insn_decode_nm_format(insn.nm_form);
        break;

    case 0b0100: // n_format, m_format, nm_format
        desc = sh2e_insn_decode_n_format(insn.n_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_m_format(insn.m_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_nm_format(insn.nm_form);
        break;

    case 0b0111: // ni_format
    case 0b1110: // ni_format
        desc = sh2e_insn_decode_ni_format(insn.ni_form);
        break;

    case 0b1000: // md_format, nd4_format, d_format, i_format
        desc = sh2e_insn_decode_md_format(insn.md_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_nd4_format(insn.nd4_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_d_format(insn.d_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_i_format(insn.i_form);

        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_msim_format(insn.z_form);
        break;

    case 0b1001: // nd8_format
    case 0b1101: // nd8_format
        desc = sh2e_insn_decode_nd8_format(insn.nd8_form);
        break;

    case 0b1010: // d12_format
    case 0b1011: // d12_format
        desc = sh2e_insn_decode_d12_format(insn.d12_form);
        break;

    case 0b1100: // d_format, i_format
        desc = sh2e_insn_decode_d_format(insn.d_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_i_format(insn.i_form);
        break;

    case 0b1111: // n_format, m_format, nm_format (FPU)
        desc = sh2e_insn_decode_n_format_fpu(insn.n_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_m_format_fpu(insn.m_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_nm_format_fpu(insn.nm_form);
        break;
    }

    return (desc != NULL) ? desc : &illegal;
}
