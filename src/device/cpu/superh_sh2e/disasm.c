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

#include "bitops.h"
#include "decode.h"
#include "decode.inc"
#include "disasm.h"

#include <ctype.h>
#include <string.h>
#include <stdint.h>


/** Register names. */

extern char const * const * sh2e_cpu_general_reg_names;
extern char const * const * sh2e_fpu_general_reg_names;



/****************************************************************************
 * SH-2E instruction disassembly
 ****************************************************************************/

static char const *
__dump_operation_name(
    char const * const format, string_t * const restrict mnemonics
) {
    string_t s_name;
    string_init(&s_name);

    char const * current = format;

    // Print first word.
    while (*current != '\0' && *current != ' ') {
        string_push(&s_name, tolower(*current));
        current++;
    }

    // Skip spaces.
    while (*current != '\0' && *current == ' ') {
        current++;
    }


    string_printf(mnemonics, "%-8s", s_name.str);
    string_done(&s_name);
    return current;
}

//

void
sh2e_insn_desc_dump_z_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    __dump_operation_name(desc->assembly, mnemonics);
}

//

void
sh2e_insn_desc_dump_n_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const rn_arg = "Rn";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, rn_arg) == format) {
            string_append(mnemonics, sh2e_cpu_general_reg_names[insn.n_form.rn]);
            format += strlen(rn_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}

//

void
sh2e_insn_desc_dump_n_format_fpu(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const frn_arg = "FRn";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, frn_arg) == format) {
            string_append(mnemonics, sh2e_fpu_general_reg_names[insn.n_form.rn]);
            format += strlen(frn_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}

//

void
sh2e_insn_desc_dump_m_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const frm_arg = "FRm";
    static char const * const rm_arg = "Rm";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, frm_arg) == format) {
            string_append(mnemonics, sh2e_fpu_general_reg_names[insn.m_form.rm]);
            format += strlen(frm_arg);
        } else if (strstr(format, rm_arg) == format) {
            string_append(mnemonics, sh2e_cpu_general_reg_names[insn.m_form.rm]);
            format += strlen(rm_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}

//

void
sh2e_insn_desc_dump_nm_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const frn_arg = "FRn";
    static char const * const frm_arg = "FRm";
    static char const * const rn_arg = "Rn";
    static char const * const rm_arg = "Rm";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, frn_arg) == format) {
            string_append(mnemonics, sh2e_fpu_general_reg_names[insn.nm_form.rn]);
            format += strlen(frn_arg);
        } else if (strstr(format, frm_arg) == format) {
            string_append(mnemonics, sh2e_fpu_general_reg_names[insn.nm_form.rm]);
            format += strlen(frm_arg);
        } else if (strstr(format, rn_arg) == format) {
            string_append(mnemonics, sh2e_cpu_general_reg_names[insn.nm_form.rn]);
            format += strlen(rn_arg);
        } else if (strstr(format, rm_arg) == format) {
            string_append(mnemonics, sh2e_cpu_general_reg_names[insn.nm_form.rm]);
            format += strlen(rm_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}

//

static inline void
sh2e_insn_desc_dump_md_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments,
    unsigned int const scale
) {
    static char const * const disp_rm_arg = "disp, Rm";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_rm_arg) == format) {
            uint32_t const disp = zero_extend_4_32(insn.md_form.d4) * scale;
            string_printf(mnemonics, "%s + %#04" PRIx32, sh2e_cpu_general_reg_names[insn.md_form.rm], disp);
            format += strlen(disp_rm_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}


void
sh2e_insn_desc_dump_md_format_movb4(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    // TODO Consider including scale in the instruction descriptor.
    sh2e_insn_desc_dump_md_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint8_t));
}


void
sh2e_insn_desc_dump_md_format_movw4(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    // TODO Consider including scale in the instruction descriptor.
    sh2e_insn_desc_dump_md_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint16_t));
}

//

static void
sh2e_insn_desc_dump_nd4_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments,
    unsigned int const scale
) {
    static char const * const disp_rn_arg = "disp, Rn";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_rn_arg) == format) {
            uint32_t const disp = zero_extend_4_32(insn.nd4_form.d4) * scale;
            string_printf(mnemonics, "%s + %#04" PRIx32, sh2e_cpu_general_reg_names[insn.nd4_form.rn], disp);
            format += strlen(disp_rn_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}


void
sh2e_insn_desc_dump_nd4_format_movb4(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_nd4_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint8_t));
}


void
sh2e_insn_desc_dump_nd4_format_movw4(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_nd4_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint16_t));
}

//

void
sh2e_insn_desc_dump_nmd_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const disp_rn_arg = "disp, Rn";
    static char const * const disp_rm_arg = "disp, Rm";
    static char const * const rn_arg = "Rn";
    static char const * const rm_arg = "Rm";

    uint32_t const disp = zero_extend_4_32(insn.nmd_form.d4) * sizeof(uint32_t);

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_rn_arg) == format) {
            string_printf(mnemonics, "%s + %#04" PRIx32, sh2e_cpu_general_reg_names[insn.nmd_form.rn], disp);
            format += strlen(disp_rn_arg);
        } else if (strstr(format, rn_arg) == format) {
            string_printf(mnemonics, "%s", sh2e_cpu_general_reg_names[insn.nmd_form.rn]);
            format += strlen(rn_arg);
        } else if (strstr(format, disp_rm_arg) == format) {
            string_printf(mnemonics, "%s + %#04" PRIx32, sh2e_cpu_general_reg_names[insn.nmd_form.rm], disp);
            format += strlen(disp_rm_arg);
        } else if (strstr(format, rm_arg) == format) {
            string_printf(mnemonics, "%s", sh2e_cpu_general_reg_names[insn.nmd_form.rm]);
            format += strlen(rm_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}

//

void
sh2e_insn_desc_dump_d_format_branch(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const disp_arg = "disp";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_arg) == format) {
            uint32_t const disp = 2 + sign_extend_8_32(insn.d_form.d8);
            uint32_t const target_addr = addr + disp * sizeof(sh2e_insn_t);
            string_printf(mnemonics, "%#010" PRIx32 " (%+di)", target_addr, (int) disp);
            format += strlen(disp_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}


void
sh2e_insn_desc_dump_d_format_mova(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const disp_pc_arg = "disp, PC";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_pc_arg) == format) {
            uint32_t const disp = zero_extend_8_32(insn.d_form.d8) * sizeof(uint32_t);
            uint32_t const target_addr = addr + 2 * sizeof(sh2e_insn_t) + disp;
            string_printf(mnemonics, "%#010" PRIx32, target_addr);
            format += strlen(disp_pc_arg);
        } else {
            string_push(mnemonics, tolower(*format));
            format += 1;
        }
    }
}


static void
sh2e_insn_desc_dump_d_format_movg(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments,
    unsigned scale
) {
    static char const * const disp_gbr_arg = "disp, GBR";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_gbr_arg) == format) {
            uint32_t const disp = zero_extend_8_32(insn.d_form.d8) * scale;
            string_printf(mnemonics, "GBR + %#05" PRIx32, disp);
            format += strlen(disp_gbr_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}


void
sh2e_insn_desc_dump_d_format_movbg(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_d_format_movg(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint8_t));
}


void
sh2e_insn_desc_dump_d_format_movwg(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_d_format_movg(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint16_t));
}


void
sh2e_insn_desc_dump_d_format_movlg(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_d_format_movg(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint32_t));
}

//

void
sh2e_insn_desc_dump_d12_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const disp_arg = "disp";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_arg) == format) {
            uint32_t const disp = 2 + sign_extend_12_32(insn.d12_form.d12);
            uint32_t const target_addr = addr + disp * sizeof(sh2e_insn_t);
            string_printf(mnemonics, "%#010" PRIx32 " (%+di)", target_addr, (int) disp);
            format += strlen(disp_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}

//

static void
sh2e_insn_desc_dump_nd8_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments,
    unsigned int const scale
) {
    static char const * const disp_pc_arg = "disp, PC";
    static char const * const rn_arg = "Rn";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, disp_pc_arg) == format) {
            uint32_t const disp = 2 + zero_extend_8_32(insn.nd8_form.d8);
            uint32_t const target_addr = addr + disp * scale;
            string_printf(mnemonics, "%#010" PRIx32, target_addr);
            format += strlen(disp_pc_arg);
        } else if (strstr(format, rn_arg) == format) {
            string_printf(mnemonics, "%s", sh2e_cpu_general_reg_names[insn.nd8_form.rn]);
            format += strlen(rn_arg);
        } else {
            string_push(mnemonics, *format);
            format++;
        }
    }
}


void
sh2e_insn_desc_dump_nd8_format_movwi(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_nd8_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint16_t));
}


void
sh2e_insn_desc_dump_nd8_format_movli(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    sh2e_insn_desc_dump_nd8_format(desc, cpu, addr, insn, mnemonics, comments, sizeof(uint32_t));
}

//

void
sh2e_insn_desc_dump_i_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const imm_arg = "#imm";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, imm_arg) == format) {
            string_printf(mnemonics, "%#04" PRIx8, insn.i_form.i8);
            format += strlen(imm_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}

//

void
sh2e_insn_desc_dump_ni_format(
    sh2e_insn_desc_t const * const restrict desc,
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, sh2e_insn_t const insn,
    string_t * const restrict mnemonics, string_t * const restrict comments
) {
    static char const * const imm_arg = "#imm";
    static char const * const rn_arg = "Rn";

    char const * format = __dump_operation_name(desc->assembly, mnemonics);
    while (*format != '\0') {
        if (strstr(format, imm_arg) == format) {
            string_printf(mnemonics, "%" PRId32, (int) sign_extend_8_32(insn.ni_form.i8));
            format += strlen(imm_arg);
        } else if (strstr(format, rn_arg) == format) {
            string_printf(mnemonics, "%s", sh2e_cpu_general_reg_names[insn.ni_form.rn]);
            format += strlen(rn_arg);
        } else {
            string_push(mnemonics, *format);
            format += 1;
        }
    }
}
