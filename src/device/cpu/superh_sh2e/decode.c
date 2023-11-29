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

#include "insn.h"
#include "disasm.h"
#include "exec.h"

#include <stddef.h>


/****************************************************************************
 * SH-2E `0-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_z_format(sh2e_insn_z_t const insn) {
    switch (insn.ic) {
    // Table A.25: 0 Format
    //   0b0000000000xxxxxx
    case 0b0000000000001000:
        static sh2e_insn_desc_t const clrt = {
            .assembly = "CLRT",
            .abstract = "0 → T",
            .exec = sh2e_insn_exec_clrt,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &clrt;

    case 0b0000000000001001:
        static sh2e_insn_desc_t const nop = {
            .assembly = "NOP",
            .abstract = "(no operation)",
            .exec = sh2e_insn_exec_nop,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &nop;

    case 0b0000000000001011:
        static sh2e_insn_desc_t const rts = {
            .assembly = "RTS",
            .abstract = "PR → PC (delayed)",
            .exec = sh2e_insn_exec_rts,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 2,
        };
        return &rts;

    case 0b0000000000011000:
        static sh2e_insn_desc_t const sett = {
            .assembly = "SETT",
            .abstract = "1 → T",
            .exec = sh2e_insn_exec_sett,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &sett;

    case 0b0000000000011001:
        static sh2e_insn_desc_t const div0u = {
            .assembly = "DIV0U",
            .abstract = "0 → M/Q/T",
            .exec = sh2e_insn_exec_div0u,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &div0u;

    case 0b0000000000011011:
        static sh2e_insn_desc_t const sleep = {
            .assembly = "SLEEP",
            .abstract = "(sleep)",
            .exec = sh2e_insn_exec_not_implemented,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 3,
        };
        return &sleep;

    case 0b0000000000101000:
        static sh2e_insn_desc_t const clrmac = {
            .assembly = "CLRMAC",
            .abstract = "0 → MACH/MACL",
            .exec = sh2e_insn_exec_clrmac,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 1,
        };
        return &clrmac;

    case 0b0000000000101011:
        static sh2e_insn_desc_t const rte = {
            .assembly = "RTE",
            .abstract = "stack area → PC/SR (delayed)",
            .exec = sh2e_insn_exec_rte,
            .disasm = sh2e_insn_desc_dump_z_format,
            .cycles = 4,
        };
        return &rte;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `n-format` decoding
 ****************************************************************************/

#define ic12(ic4, ic8) (((ic4) << 8) | (ic8))

static sh2e_insn_desc_t const *
sh2e_insn_decode_n_format(sh2e_insn_n_t const insn) {
    uint16_t const ic = ic12(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.26: Direct Register
    case ic12(0b0100, 0b00010101):
        static sh2e_insn_desc_t const cmppl = {
            .assembly = "CMP/PL Rn",
            .abstract = "Rn > 0 → T (signed)",
            .exec = sh2e_insn_exec_cmppl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &cmppl;

    case ic12(0b0100, 0b00010001):
        static sh2e_insn_desc_t const cmppz = {
            .assembly = "CMP/PZ Rn",
            .abstract = "Rn >= 0 → T (signed)",
            .exec = sh2e_insn_exec_cmppz,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &cmppz;

    case ic12(0b0100, 0b00010000):
        static sh2e_insn_desc_t const dt = {
            .assembly = "DT Rn",
            .abstract = "(Rn – 1) → Rn, (Rn == 0) → T",
            .exec = sh2e_insn_exec_dt,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &dt;

    case ic12(0b0000, 0b00101001):
        static sh2e_insn_desc_t const movt = {
            .assembly = "MOVT Rn",
            .abstract = "T → Rn",
            .exec = sh2e_insn_exec_movt,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &movt;

    case ic12(0b0100, 0b00000100):
        static sh2e_insn_desc_t const rotl = {
            .assembly = "ROTL Rn",
            .abstract = "T ← Rn ← MSB",
            .exec = sh2e_insn_exec_rotl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotl;

    case ic12(0b0100, 0b00000101):
        static sh2e_insn_desc_t const rotr = {
            .assembly = "ROTR Rn",
            .abstract = "LSB → Rn → T",
            .exec = sh2e_insn_exec_rotr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotr;

    case ic12(0b0100, 0b00100100):
        static sh2e_insn_desc_t const rotcl = {
            .assembly = "ROTCL Rn",
            .abstract = "T ← Rn ← T",
            .exec = sh2e_insn_exec_rotcl,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotcl;

    case ic12(0b0100, 0b00100101):
        static sh2e_insn_desc_t const rotcr = {
            .assembly = "ROTCR Rn",
            .abstract = "T → Rn → T",
            .exec = sh2e_insn_exec_rotcr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &rotcr;

    case ic12(0b0100, 0b00100000):
        static sh2e_insn_desc_t const shal = {
            .assembly = "SHAL Rn",
            .abstract = "T ← Rn ← 0",
            .exec = sh2e_insn_exec_shal,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shal;

    case ic12(0b0100, 0b00100001):
        static sh2e_insn_desc_t const shar = {
            .assembly = "SHAR Rn",
            .abstract = "MSB → Rn → T",
            .exec = sh2e_insn_exec_shar,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shar;

    case ic12(0b0100, 0b00000000):
        static sh2e_insn_desc_t const shll = {
            .assembly = "SHLL Rn",
            .abstract = "T ← Rn ← 0",
            .exec = sh2e_insn_exec_shll,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll;

    case ic12(0b0100, 0b00000001):
        static sh2e_insn_desc_t const shlr = {
            .assembly = "SHLR Rn",
            .abstract = "0 → Rn → T",
            .exec = sh2e_insn_exec_shlr,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr;

    // SHLL scale:    0b00xx1000
    case ic12(0b0100, 0b00001000):
        static sh2e_insn_desc_t const shll2 = {
            .assembly = "SHLL2 Rn",
            .abstract = "(Rn << 2) → Rn",
            .exec = sh2e_insn_exec_shll2,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll2;

    case ic12(0b0100, 0b00011000):
        static sh2e_insn_desc_t const shll8 = {
            .assembly = "SHLL8 Rn",
            .abstract = "(Rn << 8) → Rn",
            .exec = sh2e_insn_exec_shll8,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll8;

    case ic12(0b0100, 0b00101000):
        static sh2e_insn_desc_t const shll16 = {
            .assembly = "SHLL16 Rn",
            .abstract = "(Rn << 16) → Rn",
            .exec = sh2e_insn_exec_shll16,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shll16;

    // SHLR scale:    0b00xx1001
    case ic12(0b0100, 0b00001001):
        static sh2e_insn_desc_t const shlr2 = {
            .assembly = "SHLR2 Rn",
            .abstract = "(Rn >> 2) → Rn",
            .exec = sh2e_insn_exec_shlr2,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr2;

    case ic12(0b0100, 0b00011001):
        static sh2e_insn_desc_t const shlr8 = {
            .assembly = "SHLR8 Rn",
            .abstract = "(Rn >> 8) → Rn",
            .exec = sh2e_insn_exec_shlr8,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr8;

    case ic12(0b0100, 0b00101001):
        static sh2e_insn_desc_t const shlr16 = {
            .assembly = "SHLR16 Rn",
            .abstract = "(Rn >> 16) → Rn",
            .exec = sh2e_insn_exec_shlr16,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &shlr16;

    // Table A.27: Direct Register (Store with Control and System Registers)
    // CPU ctl reg#:  0b00xx0010
    case ic12(0b0000, 0b00000010):
        static sh2e_insn_desc_t const stc_sr = {
            .assembly = "STC SR, Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_sr;

    case ic12(0b0000, 0b00010010):
        static sh2e_insn_desc_t const stc_gbr = {
            .assembly = "STC GBR, Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_gbr;

    case ic12(0b0000, 0b00110010):
        static sh2e_insn_desc_t const stc_vbr = {
            .assembly = "STC VBR, Rn",
            .exec = sh2e_insn_exec_stc_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stc_vbr;

    // FPU sys reg#:  0b01xx1010
    case ic12(0b0000, 0b01011010):
        static sh2e_insn_desc_t const sts_fpul = {
            .assembly = "STS FPUL, Rn",
            .exec = sh2e_insn_exec_sts_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &sts_fpul;

    case ic12(0b0000, 0b01101010):
        static sh2e_insn_desc_t const sts_fpscr = {
            .assembly = "STS FPSCR, Rn",
            .exec = sh2e_insn_exec_sts_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &sts_fpscr;

    // CPU sys reg#:  0b00xx1010
    case ic12(0b0000, 0b00001010):
        static sh2e_insn_desc_t const sts_mach = {
            .assembly = "STS MACH, Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_mach;

    case ic12(0b0000, 0b00011010):
        static sh2e_insn_desc_t const sts_macl = {
            .assembly = "STS MACL, Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_macl;

    case ic12(0b0000, 0b00101010):
        static sh2e_insn_desc_t const sts_pr = {
            .assembly = "STS PR, Rn",
            .exec = sh2e_insn_exec_sts_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &sts_pr;

    // Table A.28: Indirect Register
    case ic12(0b0100, 0b00011011):
        static sh2e_insn_desc_t const tas = {
            .assembly = "TAS.B @Rn",
            .exec = sh2e_insn_exec_tas,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .bus_lock = true,
        };
        return &tas;

    // Table A.29: Indirect Pre-Decrement Register
    // CPU ctl reg#:  0b00xx0011
    case ic12(0b0100, 0b00000011):
        static sh2e_insn_desc_t const stcm_sr = {
            .assembly = "STC.L SR, @-Rn",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_sr;

    case ic12(0b0100, 0b00010011):
        static sh2e_insn_desc_t const stcm_gbr = {
            .assembly = "STC.L GBR, @-Rn",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_gbr;

    case ic12(0b0100, 0b00100011):
        static sh2e_insn_desc_t const stcm_vbr = {
            .assembly = "STC.L VBR, @-Rn",
            .exec = sh2e_insn_exec_stcm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stcm_vbr;

    // FPU sys reg#:  0b01xx0010
    case ic12(0b0100, 0b01010010):
        static sh2e_insn_desc_t const stsm_fpul = {
            .assembly = "STS.L FPUL, @-Rn",
            .exec = sh2e_insn_exec_stsm_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &stsm_fpul;

    case ic12(0b0100, 0b01100010):
        static sh2e_insn_desc_t const stsm_fpscr = {
            .assembly = "STS.L FPSCR, @-Rn",
            .exec = sh2e_insn_exec_stsm_fpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
        };
        return &stsm_fpscr;

    // CPU sys reg#:  0b00xx0010
    case ic12(0b0100, 0b00000010):
        static sh2e_insn_desc_t const stsm_mach = {
            .assembly = "STS.L MACH, @-Rn",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_mach;

    case ic12(0b0100, 0b00010010):
        static sh2e_insn_desc_t const stsm_macl = {
            .assembly = "STS.L MACL, @-Rn",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_macl;

    case ic12(0b0100, 0b00100010): // STS.L PR, @-Rn
        static sh2e_insn_desc_t const stsm_pr = {
            .assembly = "STS.L PR, @-Rn",
            .exec = sh2e_insn_exec_stsm_cpu,
            .disasm = sh2e_insn_desc_dump_n_format,
            .cycles = 1,
            .disable_interrupts = true,
        };
        return &stsm_pr;
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_n_format_fpu(sh2e_insn_n_t const insn) {
    switch (insn.ic_l) {
    // Table A.30: Floating-Point Instruction
    //   0bxxxx1101
    case 0b00001101:
        static sh2e_insn_desc_t const fsts = {
            .assembly = "FSTS FPUL, FRn",
            .abstract = "FPUL → FRn",
            .exec = sh2e_insn_exec_fsts,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fsts;

    case 0b00101101:
        static sh2e_insn_desc_t const ffloat = {
            .assembly = "FLOAT FPUL, FRn",
            .abstract = "(float) FPUL → FRn",
            .exec = sh2e_insn_exec_float,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &ffloat;

    case 0b01001101:
        static sh2e_insn_desc_t const fneg = {
            .assembly = "FNEG FRn",
            .abstract = "–FRn → FRn",
            .exec = sh2e_insn_exec_fneg,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fneg;

    case 0b01011101:
        static sh2e_insn_desc_t const fabs = {
            .assembly = "FABS FRn",
            .abstract = "|FRn| → FRn",
            .exec = sh2e_insn_exec_fabs,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fabs;

    case 0b10001101:
        static sh2e_insn_desc_t const fldi0 = {
            .assembly = "FLDI0 FRn",
            .abstract = "0.0 → FRn",
            .exec = sh2e_insn_exec_fldi0,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fldi0;

    case 0b10011101:
        static sh2e_insn_desc_t const fldi1 = {
            .assembly = "FLDI1 FRn",
            .abstract = "1.0 → FRn",
            .exec = sh2e_insn_exec_fldi1,
            .disasm = sh2e_insn_desc_dump_n_format_fpu,
            .cycles = 1,
        };
        return &fldi1;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `m-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_m_format(sh2e_insn_m_t const insn) {
    uint16_t const ic = ic12(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.31: Direct Register (Load from Control and System Registers)
    // CPU ctl reg#:  0b00xx1110
    case ic12(0b0100, 0b00001110): // LDC Rm, SR
    case ic12(0b0100, 0b00011110): // LDC Rm, GBR
    case ic12(0b0100, 0b00101110): // LDC Rm, VBR

    // FPU sys reg#:  0b01xx1010
    case ic12(0b0100, 0b01011010): // LDS Rm, FPUL
    case ic12(0b0100, 0b01101010): // LDS Rm, FPSCR

    // CPU sys reg#:  0b00xx1010
    case ic12(0b0100, 0b00001010): // LDS Rm, MACH
    case ic12(0b0100, 0b00011010): // LDS Rm, MACL
    case ic12(0b0100, 0b00101010): // LDS Rm, PR
        alert("sh2e_insn_decode_m_format not implemented for ic12=%#04x", ic);
        break;

    // Table A.32: Indirect Register
    case ic12(0b0100, 0b00101011):
        static sh2e_insn_desc_t const jmp = {
            .assembly = "JMP @Rm",
            .abstract = "Rm → PC (delayed)",
            .exec = sh2e_insn_exec_jmp,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 2,
        };
        return &jmp;

    case ic12(0b0100, 0b00001011):
        static sh2e_insn_desc_t const jsr = {
            .assembly = "JSR @Rm",
            .abstract = "PC + 4 → PR, Rm → PC (delayed)",
            .exec = sh2e_insn_exec_jsr,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 2,
        };
        return &jsr;

    // Table A.33: Indirect Post-Increment Register
    // CPU ctl reg#:  0b00xx0111
    case ic12(0b0100, 0b00000111): // LDC.L @Rm+, SR
    case ic12(0b0100, 0b00010111): // LDC.L @Rm+, GBR
    case ic12(0b0100, 0b00100111): // LDC.L @Rm+, VBR

    // FPU sys reg#:  0b01xx0110
    case ic12(0b0100, 0b01010110): // LDS.L @Rm+, FPUL
    case ic12(0b0100, 0b01100110): // LDS.L @Rm+, FPSCR

    // CPU sys reg#:  0b00xx0110
    case ic12(0b0100, 0b00000110): // LDS.L @Rm+, MACH
    case ic12(0b0100, 0b00010110): // LDS.L @Rm+, MACL
    case ic12(0b0100, 0b00100110): // LDS.L @Rm+, PR
        alert("sh2e_insn_decode_m_format not implemented for ic12=%#04x", ic);
        break;

    // Table A.34: PC Relative Addressing with Rn
    case ic12(0b0000, 0b00100011):
        static sh2e_insn_desc_t const braf = {
            .assembly = "BRAF Rm",
            .abstract = "PC + 4 + Rm → PC (delayed)",
            .exec = sh2e_insn_exec_braf,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 2,
        };
        return &braf;

    case ic12(0b0000, 0b00000011):
        static sh2e_insn_desc_t const bsrf = {
            .assembly = "BSRF Rm",
            .abstract = "PC + 4 → PR, PC + 4 + Rm → PC (delayed)",
            .exec = sh2e_insn_exec_bsrf,
            .disasm = sh2e_insn_desc_dump_m_format,
            .disable_interrupts = true,
            .cycles = 2,
        };
        return &bsrf;
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_m_format_fpu(sh2e_insn_m_t const insn) {
    switch (insn.ic_l) {
    // Table A.35: Floating-Point Instructions
    case 0b00011101:
        static sh2e_insn_desc_t const flds = {
            .assembly = "FLDS FRm, FPUL",
            .abstract = "FRm → FPUL",
            .exec = sh2e_insn_exec_flds,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &flds;

    case 0b00111101:
        static sh2e_insn_desc_t const ftrc = {
            .assembly = "FTRC FRm, FPUL",
            .abstract = "(long) FRm → FPUL",
            .exec = sh2e_insn_exec_ftrc,
            .disasm = sh2e_insn_desc_dump_m_format,
            .cycles = 1,
        };
        return &ftrc;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `nm-format` decoding
 ****************************************************************************/

#define ic8(ic8, ic4) (((ic8) << 4) | (ic4))

static sh2e_insn_desc_t const *
sh2e_insn_decode_nm_format(sh2e_insn_nm_t const insn) {
    uint_fast8_t const ic = ic8(insn.ic_h, insn.ic_l);
    switch (ic) {
    // Table A.36: Direct Register
    case ic8(0b0011, 0b1100):
        static sh2e_insn_desc_t const add = {
            .assembly = "ADD Rm, Rn",
            .abstract = "Rn + Rm → Rn",
            .exec = sh2e_insn_exec_add,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &add;

    case ic8(0b0011, 0b1110):
        static sh2e_insn_desc_t const addc = {
            .assembly = "ADDC Rm, Rn",
            .abstract = "Rn + Rm + T → Rn (unsigned), carry → T",
            .exec = sh2e_insn_exec_addc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &addc;

    case ic8(0b0011, 0b1111):
        static sh2e_insn_desc_t const addv = {
            .assembly = "ADDV Rm, Rn",
            .abstract = "Rn + Rm → Rn (signed), overflow → T",
            .exec = sh2e_insn_exec_addv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &addv;

    case ic8(0b0010, 0b1001):
        static sh2e_insn_desc_t const and = {
            .assembly = "AND Rm, Rn",
            .abstract = "Rn & Rm → Rn",
            .exec = sh2e_insn_exec_and,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &and;

    case ic8(0b0011, 0b0000):
        static sh2e_insn_desc_t const cmpeq = {
            .assembly = "CMP/EQ Rm, Rn",
            .abstract = "Rn == Rm → T",
            .exec = sh2e_insn_exec_cmpeq,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpeq;

    case ic8(0b0011, 0b0010):
        static sh2e_insn_desc_t const cmphs = {
            .assembly = "CMP/HS Rm, Rn",
            .abstract = "Rn >= Rm → T (unsigned)",
            .exec = sh2e_insn_exec_cmphs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmphs;

    case ic8(0b0011, 0b0011):
        static sh2e_insn_desc_t const cmpge = {
            .assembly = "CMP/GE Rm, Rn",
            .abstract = "Rn >= Rm → T (signed)",
            .exec = sh2e_insn_exec_cmpge,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpge;

    case ic8(0b0011, 0b0110):
        static sh2e_insn_desc_t const cmphi = {
            .assembly = "CMP/HI Rm, Rn",
            .abstract = "Rn > Rm → T (unsigned)",
            .exec = sh2e_insn_exec_cmphi,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmphi;

    case ic8(0b0011, 0b0111):
        static sh2e_insn_desc_t const cmpgt = {
            .assembly = "CMP/GT Rm, Rn",
            .abstract = "Rn > Rm → T (signed)",
            .exec = sh2e_insn_exec_cmpgt,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpgt;

    case ic8(0b0010, 0b1100):
        static sh2e_insn_desc_t const cmpstr = {
            .assembly = "CMP/STR Rm, Rn",
            .abstract = "any byte in Rn equals corresponding byte in Rm → T",
            .exec = sh2e_insn_exec_cmpstr,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &cmpstr;

    case ic8(0b0011, 0b0100):
        static sh2e_insn_desc_t const div1 = {
            .assembly = "DIV1 Rm, Rn",
            .abstract = "Rn ÷ Rm → T (1 step)",
            .exec = sh2e_insn_exec_div1,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &div1;

    case ic8(0b0010, 0b0111):
        static sh2e_insn_desc_t const div0s = {
            .assembly = "DIV0S Rm, Rn",
            .abstract = "Rn{31} → Q, Rm{31} → M, Q ^ M → T",
            .exec = sh2e_insn_exec_div0s,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &div0s;

    case ic8(0b0011, 0b1101):
        static sh2e_insn_desc_t const dmulsl = {
            .assembly = "DMULS.L Rm, Rn",
            .abstract = "Rn × Rm → MACH:MACL (signed)",
            .exec = sh2e_insn_exec_not_implemented,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &dmulsl;

    case ic8(0b0011, 0b0101):
        static sh2e_insn_desc_t const dmulul = {
            .assembly = "DMULU.L Rm, Rn",
            .abstract = "Rn × Rm → MACH:MACL (unsigned)",
            .exec = sh2e_insn_exec_not_implemented,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &dmulul;

    case ic8(0b0110, 0b1110):
        static sh2e_insn_desc_t const extsb = {
            .assembly = "EXTS.B Rm, Rn",
            .abstract = "SE(Rm{7:0}) → Rn",
            .exec = sh2e_insn_exec_extsb,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extsb;

    case ic8(0b0110, 0b1111):
        static sh2e_insn_desc_t const extsw = {
            .assembly = "EXTS.W Rm, Rn",
            .abstract = "SE(Rm{15:0}) → Rn",
            .exec = sh2e_insn_exec_extsw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extsw;

    case ic8(0b0110, 0b1100):
        static sh2e_insn_desc_t const extub = {
            .assembly = "EXTU.B Rm, Rn",
            .abstract = "ZE(Rm{7:0}) → Rn",
            .exec = sh2e_insn_exec_extub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extub;

    case ic8(0b0110, 0b1101):
        static sh2e_insn_desc_t const extuw = {
            .assembly = "EXTU.W Rm, Rn",
            .abstract = "ZE(Rm{15:0}) → Rn",
            .exec = sh2e_insn_exec_extuw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &extuw;

    case ic8(0b0110, 0b0011):
        static sh2e_insn_desc_t const mov = {
            .assembly = "MOV Rm, Rn",
            .abstract = "Rm → Rn",
            .exec = sh2e_insn_exec_mov,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &mov;

    case ic8(0b0000, 0b0111):
        static sh2e_insn_desc_t const mull = {
            .assembly = "MUL.L Rm, Rn",
            .abstract = "Rn × Rm → MACL",
            .exec = sh2e_insn_exec_mull,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 2, // 2-4 cycles
        };
        return &mull;

    case ic8(0b0010, 0b1111):
        static sh2e_insn_desc_t const mulsw = {
            .assembly = "MULS.W Rm, Rn",
            .abstract = "Rn{15:0} × Rm{15:0} → MACL (signed)",
            .exec = sh2e_insn_exec_mulsw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1, // 1-3 cycles
        };
        return &mulsw;

    case ic8(0b0010, 0b1110):
        static sh2e_insn_desc_t const muluw = {
            .assembly = "MULU.W Rm, Rn",
            .abstract = "Rn{15:0} × Rm{15:0} → MACL (unsigned)",
            .exec = sh2e_insn_exec_muluw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1, // 1-3 cycles
        };
        return &muluw;

    case ic8(0b0110, 0b1011):
        static sh2e_insn_desc_t const neg = {
            .assembly = "NEG Rm, Rn",
            .abstract = "0 – Rm → Rn",
            .exec = sh2e_insn_exec_neg,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &neg;

    case ic8(0b0110, 0b1010):
        static sh2e_insn_desc_t const negc = {
            .assembly = "NEGC Rm, Rn",
            .abstract = "0 – Rm – T → Rn, borrow → T",
            .exec = sh2e_insn_exec_negc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &negc;

    case ic8(0b0110, 0b0111):
        static sh2e_insn_desc_t const not = {
            .assembly = "NOT Rm, Rn",
            .abstract = "~Rm → Rn",
            .exec = sh2e_insn_exec_not,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &not;

    case ic8(0b0010, 0b1011):
        static sh2e_insn_desc_t const or = {
            .assembly = "OR Rm, Rn",
            .abstract = "Rn | Rm → Rn",
            .exec = sh2e_insn_exec_or,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &or;

    case ic8(0b0011, 0b1000):
        static sh2e_insn_desc_t const sub = {
            .assembly = "SUB Rm, Rn",
            .abstract = "Rn – Rm → Rn",
            .exec = sh2e_insn_exec_sub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &sub;

    case ic8(0b0011, 0b1010):
        static sh2e_insn_desc_t const subc = {
            .assembly = "SUBC Rm, Rn",
            .abstract = "Rn - Rm - T → Rn (unsigned), borrow → T",
            .exec = sh2e_insn_exec_subc,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &subc;

    case ic8(0b0011, 0b1011):
        static sh2e_insn_desc_t const subv = {
            .assembly = "SUBV Rm, Rn",
            .abstract = "Rn - Rm → Rn (signed), underflow → T",
            .exec = sh2e_insn_exec_subv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &subv;

    case ic8(0b0110, 0b1000):
        static sh2e_insn_desc_t const swapb = {
            .assembly = "SWAP.B Rm, Rn",
            .abstract = "Rm{31:16}:Rm{7:0}:Rm{15:8} → Rn",
            .exec = sh2e_insn_exec_swapb,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &swapb;

    case ic8(0b0110, 0b1001):
        static sh2e_insn_desc_t const swapw = {
            .assembly = "SWAP.B Rm, Rn",
            .abstract = "Rm{15:0}:Rm{31:16} → Rn",
            .exec = sh2e_insn_exec_swapw,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &swapw;

    case ic8(0b0010, 0b1000):
        static sh2e_insn_desc_t const tst = {
            .assembly = "TST Rm, Rn",
            .abstract = "(Rn & Rm) == 0 → T",
            .exec = sh2e_insn_exec_tst,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &tst;

    case ic8(0b0010, 0b1010):
        static sh2e_insn_desc_t const xor = {
            .assembly = "XOR Rm, Rn",
            .abstract = "Rn ^ Rm → Rn",
            .exec = sh2e_insn_exec_xor,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &xor;

    case ic8(0b0010, 0b1101):
        static sh2e_insn_desc_t const xtrct = {
            .assembly = "XTRCT Rm, Rn",
            .abstract = "Rm{15:0}:Rn{31:16} → Rn",
            .exec = sh2e_insn_exec_xtrct,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &xtrct;

    // Table A.37: Indirect Register
    case ic8(0b0010, 0b0000):
        static sh2e_insn_desc_t const movbs = {
            .assembly = "MOV.B Rm, @Rn",
            .abstract = "Rm{7:0} → [Rn]",
            .exec = sh2e_insn_exec_movbs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbs;

    case ic8(0b0010, 0b0001):
        static sh2e_insn_desc_t const movws = {
            .assembly = "MOV.W Rm, @Rn",
            .abstract = "Rm{15:0} → [Rn]",
            .exec = sh2e_insn_exec_movws,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movws;

    case ic8(0b0010, 0b0010):
        static sh2e_insn_desc_t const movls = {
            .assembly = "MOV.L Rm, @Rn",
            .abstract = "Rm → [Rn]",
            .exec = sh2e_insn_exec_movls,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movls;

    case ic8(0b0110, 0b0000):
        static sh2e_insn_desc_t const movbl = {
            .assembly = "MOV.B @Rm, Rn",
            .abstract = "SE([Rm]{7:0}) → Rn",
            .exec = sh2e_insn_exec_movbl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbl;

    case ic8(0b0110, 0b0001):
        static sh2e_insn_desc_t const movwl = {
            .assembly = "MOV.W @Rm, Rn",
            .abstract = "SE([Rm]{15:0}) → Rn",
            .exec = sh2e_insn_exec_movwl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwl;

    case ic8(0b0110, 0b0010):
        static sh2e_insn_desc_t const movll = {
            .assembly = "MOV.L @Rm, Rn",
            .abstract = "[Rm] → Rn",
            .exec = sh2e_insn_exec_movll,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movll;

    // Table A.38: Indirect Post-Increment Register (Multiply/Accumulate Operation)
    case ic8(0b0000, 0b1111):
        static sh2e_insn_desc_t const macl = {
            .assembly = "MAC.L @Rm+, @Rn+",
            .abstract = "[Rn]{15:0} × [Rm]{15:0} + MAC → MAC (signed), Rn + 2 → Rn, Rm + 2 → Rm",
            .exec = sh2e_insn_exec_not_implemented,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 3, // 2 to 4
        };
        return &macl;

    case ic8(0b0100, 0b1111):
        static sh2e_insn_desc_t const macw = {
            .assembly = "MAC.W @Rm+, @Rn+",
            .abstract = "[Rn] × [Rm] + MAC → MAC (signed), Rn + 4 → Rn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_not_implemented,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 3, // 2 to 3
        };
        return &macw;

    // Table A.39: Indirect Post-Increment Register
    case ic8(0b0110, 0b0100):
        static sh2e_insn_desc_t const movbp = {
            .assembly = "MOV.B @Rm+, Rn",
            .abstract = "SE([Rm]{7:0}) → Rn, Rm + 1 → Rm",
            .exec = sh2e_insn_exec_movbp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbp;

    case ic8(0b0110, 0b0101):
        static sh2e_insn_desc_t const movwp = {
            .assembly = "MOV.W @Rm+, Rn",
            .abstract = "SE([Rm]{15:0}) → Rn, Rm + 2 → Rm",
            .exec = sh2e_insn_exec_movwp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwp;

    case ic8(0b0110, 0b0110):
        static sh2e_insn_desc_t const movlp = {
            .assembly = "MOV.L @Rm+, Rn",
            .abstract = "[Rm] → Rn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_movlp,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movlp;

    // Table A.40: Indirect Pre-Decrement Register
    case ic8(0b0010, 0b0100):
        static sh2e_insn_desc_t const movbm = {
            .assembly = "MOV.B Rm, @–Rn",
            .abstract = "Rn - 1 → Rn, Rm{7:0} → [Rn]",
            .exec = sh2e_insn_exec_movbm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbm;

    case ic8(0b0010, 0b0101):
        static sh2e_insn_desc_t const movwm = {
            .assembly = "MOV.W Rm, @–Rn",
            .abstract = "Rn - 2 → Rn, Rm{15:0} → [Rn]",
            .exec = sh2e_insn_exec_movwm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwm;

    case ic8(0b0010, 0b0110):
        static sh2e_insn_desc_t const movlm = {
            .assembly = "MOV.L Rm, @–Rn",
            .abstract = "Rn - 4 → Rn, Rm → [Rn]",
            .exec = sh2e_insn_exec_movlm,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movlm;

    // Table A.41: Indirect Indexed Register
    case ic8(0b0000, 0b0100):
        static sh2e_insn_desc_t const movbs0 = {
            .assembly = "MOV.B Rm, @(R0, Rn)",
            .abstract = "Rm{7:0} → [R0 + Rn]",
            .exec = sh2e_insn_exec_movbs0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbs0;

    case ic8(0b0000, 0b0101):
        static sh2e_insn_desc_t const movws0 = {
            .assembly = "MOV.W Rm, @(R0, Rn)",
            .abstract = "Rm{15:0} → [R0 + Rn]",
            .exec = sh2e_insn_exec_movws0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movws0;

    case ic8(0b0000, 0b0110):
        static sh2e_insn_desc_t const movls0 = {
            .assembly = "MOV.L Rm, @(R0, Rn)",
            .abstract = "Rm → [R0 + Rn]",
            .exec = sh2e_insn_exec_movls0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movls0;

    case ic8(0b0000, 0b1100):
        static sh2e_insn_desc_t const movbl0 = {
            .assembly = "MOV.B @(R0, Rm), Rn",
            .abstract = "SE([R0 + Rm]{7:0}) → Rn",
            .exec = sh2e_insn_exec_movbl0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movbl0;

    case ic8(0b0000, 0b1101):
        static sh2e_insn_desc_t const movwl0 = {
            .assembly = "MOV.W @(R0, Rm), Rn",
            .abstract = "SE([R0 + Rm]{15:0}) → Rn",
            .exec = sh2e_insn_exec_movwl0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movwl0;

    case ic8(0b0000, 0b1110):
        static sh2e_insn_desc_t const movll0 = {
            .assembly = "MOV.L @(R0, Rm), Rn",
            .abstract = "[R0 + Rm] → Rn",
            .exec = sh2e_insn_exec_movll0,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &movll0;
    }

    return NULL;
}

//

static sh2e_insn_desc_t const *
sh2e_insn_decode_nm_format_fpu(sh2e_insn_nm_t const insn) {
    switch (insn.ic_l) {
    // Table A.42: Floating Point Instructions
    case 0b0000:
        static sh2e_insn_desc_t const fadd = {
            .assembly = "FADD FRm, FRn",
            .abstract = "FRn + FRm → FRn",
            .exec = sh2e_insn_exec_fadd,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fadd;

    case 0b0100:
        static sh2e_insn_desc_t const fcmpeq = {
            .assembly = "FCMP/EQ FRm, FRn",
            .abstract = "FRn == FRm → T",
            .exec = sh2e_insn_exec_fcmpeq,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fcmpeq;

    case 0b0101:
        static sh2e_insn_desc_t const fcmpgt = {
            .assembly = "FCMP/GT FRm, FRn",
            .abstract = "FRn > FRm → T",
            .exec = sh2e_insn_exec_fcmpgt,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fcmpgt;

    case 0b0011:
        static sh2e_insn_desc_t const fdiv = {
            .assembly = "FDIV FRm, FRn",
            .abstract = "FRn ÷ FRm → FRn",
            .exec = sh2e_insn_exec_fdiv,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 13,
        };
        return &fdiv;

    case 0b1110:
        static sh2e_insn_desc_t const fmac = {
            .assembly = "FMAC FR0, FRm, FRn",
            .abstract = "FR0 × FRm + FRn → FRn",
            .exec = sh2e_insn_exec_fmac,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmac;

    case 0b1100:
        static sh2e_insn_desc_t const fmov = {
            .assembly = "FMOV FRm, FRn",
            .abstract = "FRm → FRn",
            .exec = sh2e_insn_exec_fmov,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmov;

    case 0b0110:
        static sh2e_insn_desc_t const fmovli = {
            .assembly = "FMOV.S @(R0, Rm), FRn",
            .abstract = "[R0 + Rm] → FRn",
            .exec = sh2e_insn_exec_fmovli,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovli;

    case 0b1001:
        static sh2e_insn_desc_t const fmovlr = {
            .assembly = "FMOV.S @Rm+, FRn",
            .abstract = "[Rm] → FRn, Rm + 4 → Rm",
            .exec = sh2e_insn_exec_fmovlr,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovlr;

    case 0b1000:
        static sh2e_insn_desc_t const fmovl = {
            .assembly = "FMOV.S @Rm, FRn",
            .abstract = "[Rm] → FRn",
            .exec = sh2e_insn_exec_fmovl,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovl;

    case 0b0111:
        static sh2e_insn_desc_t const fmovsi = {
            .assembly = "FMOV.S FRm, @(R0, Rn)",
            .abstract = "FRm → [R0 + Rn]",
            .exec = sh2e_insn_exec_fmovsi,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovsi;

    case 0b1011:
        static sh2e_insn_desc_t const fmovss = {
            .assembly = "FMOV.S FRm, @-Rn",
            .abstract = "Rn - 4 → Rn, FRm → [Rn]",
            .exec = sh2e_insn_exec_fmovss,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovss;

    case 0b1010:
        static sh2e_insn_desc_t const fmovs = {
            .assembly = "FMOV.S FRm, @Rn",
            .abstract = "FRm → [Rn]",
            .exec = sh2e_insn_exec_fmovs,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmovs;

    case 0b0010:
        static sh2e_insn_desc_t const fmul = {
            .assembly = "FMUL FRm, FRn",
            .abstract = "FRn × FRm → FRn",
            .exec = sh2e_insn_exec_fmul,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fmul;

    case 0b0001:
        static sh2e_insn_desc_t const fsub = {
            .assembly = "FSUB FRm, FRn",
            .abstract = "FRn – FRm → FRn",
            .exec = sh2e_insn_exec_fsub,
            .disasm = sh2e_insn_desc_dump_nm_format,
            .cycles = 1,
        };
        return &fsub;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `md-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_md_format(sh2e_insn_md_t const insn) {
    switch (insn.ic) {
    // Table A.43: Indirect register addressing with displacement
    case 0b10000100:
        static sh2e_insn_desc_t const movbl4 = {
            .assembly = "MOV.B @(disp, Rm), R0",
            .abstract = "SE([Rm + ZE(disp)]) → R0",
            .exec = sh2e_insn_exec_movbl4,
            .disasm = sh2e_insn_desc_dump_md_format_movb4,
            .cycles = 1,
        };
        return &movbl4;

    case 0b10000101:
        static sh2e_insn_desc_t const movwl4 = {
            .assembly = "MOV.W @(disp, Rm), R0",
            .abstract = "SE([Rm + ZE(disp) × 2]) → R0",
            .exec = sh2e_insn_exec_movwl4,
            .disasm = sh2e_insn_desc_dump_md_format_movw4,
            .cycles = 1,
        };
        return &movwl4;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `nd4-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nd4_format(sh2e_insn_nd4_t const insn) {
    switch (insn.ic) {
    // Table A.44: Indirect register addressing with displacement
    case 0b10000000:
        static sh2e_insn_desc_t const movbs4 = {
            .assembly = "MOV.B R0, @(disp, Rn)",
            .abstract = "R0{7:0} → [Rn + ZE(disp)]",
            .exec = sh2e_insn_exec_movbs4,
            .disasm = sh2e_insn_desc_dump_nd4_format_movb4,
            .cycles = 1,
        };
        return &movbs4;

    case 0b10000001:
        static sh2e_insn_desc_t const movws4 = {
            .assembly = "MOV.W R0, @(disp, Rn)",
            .abstract = "R0{15:0} → [Rn + ZE(disp) × 2]",
            .exec = sh2e_insn_exec_movws4,
            .disasm = sh2e_insn_desc_dump_nd4_format_movw4,
            .cycles = 1,
        };
        return &movws4;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `nmd-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nmd_format(sh2e_insn_nmd_t const insn) {
    switch (insn.ic) {
    // Table A.45: Indirect register addressing with displacement
    case 0b0001:
        static sh2e_insn_desc_t const movls4 = {
            .assembly = "MOV.L Rm, @(disp, Rn)",
            .abstract = "Rm → [Rn + ZE(disp) × 4]",
            .exec = sh2e_insn_exec_movls4,
            .disasm = sh2e_insn_desc_dump_nmd_format,
            .cycles = 1,
        };
        return &movls4;

    case 0b0101:
        static sh2e_insn_desc_t const movll4 = {
            .assembly = "MOV.L @(disp, Rm), Rn",
            .abstract = "[Rm + ZE(disp) × 4] → Rn",
            .exec = sh2e_insn_exec_movll4,
            .disasm = sh2e_insn_desc_dump_nmd_format,
            .cycles = 1,
        };
        return &movll4;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `d-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_d_format(sh2e_insn_d_t const insn) {
    switch (insn.ic) {
    // Table A.46: Indirect GBR addressing with Displacement
    case 0b11000000:
        static sh2e_insn_desc_t const movblg = {
            .assembly = "MOV.B R0, @(disp, GBR)",
            .abstract = "R0{7:0} → [GBR + ZE(disp)]",
            .exec = sh2e_insn_exec_movblg,
            .disasm = sh2e_insn_desc_dump_d_format_movbg,
            .cycles = 1,
        };
        return &movblg;

    case 0b11000001:
        static sh2e_insn_desc_t const movwlg = {
            .assembly = "MOV.W R0, @(disp, GBR)",
            .abstract = "R0{15:0} → [GBR + ZE(disp) × 2]",
            .exec = sh2e_insn_exec_movwlg,
            .disasm = sh2e_insn_desc_dump_d_format_movwg,
            .cycles = 1,
        };
        return &movwlg;

    case 0b11000010:
        static sh2e_insn_desc_t const movllg = {
            .assembly = "MOV.L R0, @(disp, GBR)",
            .abstract = "R0 → [GBR + ZE(disp) × 4]",
            .exec = sh2e_insn_exec_movllg,
            .disasm = sh2e_insn_desc_dump_d_format_movlg,
            .cycles = 1,
        };
        return &movllg;

    case 0b11000100:
        static sh2e_insn_desc_t const movbsg = {
            .assembly = "MOV.B @(disp, GBR), R0",
            .abstract = "SE([GBR + ZE(disp)]) → R0",
            .exec = sh2e_insn_exec_movbsg,
            .disasm = sh2e_insn_desc_dump_d_format_movbg,
            .cycles = 1,
        };
        return &movbsg;

    case 0b11000101:
        static sh2e_insn_desc_t const movwsg = {
            .assembly = "MOV.W @(disp, GBR), R0",
            .abstract = "SE([GBR + ZE(disp) × 2]) → R0",
            .exec = sh2e_insn_exec_movwsg,
            .disasm = sh2e_insn_desc_dump_d_format_movwg,
            .cycles = 1,
        };
        return &movwsg;

    case 0b11000110:
        static sh2e_insn_desc_t const movlsg = {
            .assembly = "MOV.L @(disp, GBR), R0",
            .abstract = "[GBR + ZE(disp) × 4] → R0",
            .exec = sh2e_insn_exec_movlsg,
            .disasm = sh2e_insn_desc_dump_d_format_movlg,
            .cycles = 1,
        };
        return &movlsg;

    // Table A.47: PC Relative with Displacement
    case 0b11000111: //
        static sh2e_insn_desc_t const mova = {
            .assembly = "MOVA @(disp, PC), R0",
            .abstract = "[PC + ZE(disp) × 4] → R0",
            .exec = sh2e_insn_exec_mova,
            .disasm = sh2e_insn_desc_dump_d_format_mova,
            .cycles = 1,
        };
        return &mova;

    // Table A.48: PC Relative addressing
    case 0b10001011:
        static sh2e_insn_desc_t const bf = {
            .assembly = "BF disp",
            .abstract = "if T = 0, PC + SE(disp) × 2 → PC",
            .exec = sh2e_insn_exec_bf,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .cycles = 1,
            .branch_cycles = 2,
        };
        return &bf;

    case 0b10001111:
        static sh2e_insn_desc_t const bfs = {
            .assembly = "BF/S disp",
            .abstract = "if T = 0, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bfs,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .disable_interrupts = true,
            .cycles = 1,
            .branch_cycles = 1,
        };
        return &bfs;

    case 0b10001001:
        static sh2e_insn_desc_t const bt = {
            .assembly = "BT disp",
            .abstract = "if T = 1, PC + 4 + SE(disp) × 2 → PC",
            .exec = sh2e_insn_exec_bt,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .cycles = 1,
            .branch_cycles = 2,
        };
        return &bt;

    case 0b10001101:
        static sh2e_insn_desc_t const bts = {
            .assembly = "BT/S disp",
            .abstract = "if T = 1, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bts,
            .disasm = sh2e_insn_desc_dump_d_format_branch,
            .disable_interrupts = true,
            .cycles = 1,
            .branch_cycles = 1,
        };
        return &bts;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `d12-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_d12_format(sh2e_insn_d12_t const insn) {
    switch (insn.ic) {
    // Table A.49: PC relative addressing
    case 0b1010:
        static sh2e_insn_desc_t const bra = {
            .assembly = "BRA disp",
            .abstract = "PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bra,
            .disasm = sh2e_insn_desc_dump_d12_format,
            .disable_interrupts = true,
            .cycles = 2,
        };
        return &bra;

    case 0b1011:
        static sh2e_insn_desc_t const bsr = {
            .assembly = "BSR disp",
            .abstract = "PC + 4 → PR, PC + 4 + SE(disp) × 2 → PC (delayed)",
            .exec = sh2e_insn_exec_bsr,
            .disasm = sh2e_insn_desc_dump_d12_format,
            .disable_interrupts = true,
            .cycles = 2,
        };
        return &bsr;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `nd8-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_nd8_format(sh2e_insn_nd8_t const insn) {
    switch (insn.ic) {
    // Table A.50: PC relative addressing with displacement
    case 0b1001:
        static sh2e_insn_desc_t const movwi = {
            .assembly = "MOV.W @(disp, PC), Rn",
            .abstract = "SE([PC + ZE(disp) × 2]) → Rn",
            .exec = sh2e_insn_exec_movwi,
            .disasm = sh2e_insn_desc_dump_nd8_format_movwi,
            .cycles = 1,
        };
        return &movwi;

    case 0b1101:
        static sh2e_insn_desc_t const movli = {
            .assembly = "MOV.L @(disp, PC), Rn",
            .abstract = "[PC + ZE(disp) × 4] → Rn",
            .exec = sh2e_insn_exec_movli,
            .disasm = sh2e_insn_desc_dump_nd8_format_movli,
            .cycles = 1,
        };
        return &movli;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `i-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_i_format(sh2e_insn_i_t const insn) {
    switch (insn.ic) {
    // Table A.51: Indirect Indexed GBR
    case 0b11001100:
        static sh2e_insn_desc_t const tstm = {
            .assembly = "TST.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] & #imm == 0 → T",
            .exec = sh2e_insn_exec_tstm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &tstm;

    case 0b11001101:
        static sh2e_insn_desc_t const andm = {
            .assembly = "AND.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] & #imm → [GBR + R0]",
            .exec = sh2e_insn_exec_andm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &andm;

    case 0b11001110:
        static sh2e_insn_desc_t const xorm = {
            .assembly = "XOR.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] ^ imm → [GBR + R0]",
            .exec = sh2e_insn_exec_xorm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &xorm;

    case 0b11001111:
        static sh2e_insn_desc_t const orm = {
            .assembly = "OR.B #imm, @(GBR, R0)",
            .abstract = "[GBR + R0] | imm → [GBR + R0]",
            .exec = sh2e_insn_exec_orm,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 3,
        };
        return &orm;

    // Table A.52: Immediate (Arithmetic Logical Operation with Direct Register)
    case 0b10001000:
        static sh2e_insn_desc_t const cmpim = {
            .assembly = "CMP/EQ #imm, R0",
            .exec = sh2e_insn_exec_cmpim,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &cmpim;

    case 0b11001000:
        static sh2e_insn_desc_t const tsti = {
            .assembly = "TST #imm, R0",
            .exec = sh2e_insn_exec_tsti,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &tsti;

    case 0b11001001:
        static sh2e_insn_desc_t const andi = {
            .assembly = "AND #imm, R0",
            .exec = sh2e_insn_exec_andi,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &andi;

    case 0b11001010:
        static sh2e_insn_desc_t const xori = {
            .assembly = "XOR #imm, R0",
            .exec = sh2e_insn_exec_xori,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &xori;

    case 0b11001011:
        static sh2e_insn_desc_t const ori = {
            .assembly = "OR #imm, R0",
            .exec = sh2e_insn_exec_ori,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 1,
        };
        return &ori;

    // Table A.53: Immediate (Specify Exception Processing Vector)
    case 0b11000011:
        static sh2e_insn_desc_t const trapa = {
            .assembly = "TRAPA #imm",
            .exec = sh2e_insn_exec_trapa,
            .disasm = sh2e_insn_desc_dump_i_format,
            .cycles = 8,
        };
        return &trapa;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E `ni-format` decoding
 ****************************************************************************/

static sh2e_insn_desc_t const *
sh2e_insn_decode_ni_format(sh2e_insn_ni_t const insn) {
    // Table A.54: Immediate addressing (direct register arithmetic operations and data transfers)
    switch (insn.ic) {
    case 0b0111:
        static sh2e_insn_desc_t const addi = {
            .assembly = "ADD #imm, Rn",
            .abstract = "Rn + SE(#imm) → Rn",
            .exec = sh2e_insn_exec_addi,
            .disasm = sh2e_insn_desc_dump_ni_format,
            .cycles = 1,
        };
        return &addi;

    case 0b1110:
        static sh2e_insn_desc_t const movi = {
            .assembly = "MOV #imm, Rn",
            .abstract = "SE(#imm) → Rn",
            .exec = sh2e_insn_exec_movi,
            .disasm = sh2e_insn_desc_dump_ni_format,
            .cycles = 1,
        };
        return &movi;
    }

    return NULL;
}


/****************************************************************************
 * SH-2E instruction decoding
 ****************************************************************************/

sh2e_insn_desc_t const *
sh2e_insn_decode(sh2e_insn_t const insn) {
    sh2e_insn_desc_t const * desc = NULL;

    // Decode using the top 4 bits.
    // See Table A.56: Operation Code Map.
    switch (insn.word >> 12) {
    case 0b0000: // z_format, n_format, nm_format
        desc = sh2e_insn_decode_z_format(insn.z_form);
        if (desc != NULL) {
            return desc;
        }

        desc = sh2e_insn_decode_n_format(insn.n_form);
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

    //

    static sh2e_insn_desc_t const illegal = {
        .assembly = "<ILLEGAL>",
        .exec = sh2e_insn_exec_illegal,
        .cycles = 1,
    };

    return (desc != NULL) ? desc : &illegal;
}
