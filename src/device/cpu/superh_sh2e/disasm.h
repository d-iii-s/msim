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

#ifndef SUPERH_SH2E_DISASM_H_
#define SUPERH_SH2E_DISASM_H_

#include "cpu.h"
#include "decode.h"
#include "insn.h"

#include "../../../utils.h"

#include <stdint.h>


// Instruction disassembly functions

extern void sh2e_insn_desc_dump_z_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_n_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_n_format_fpu(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_m_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nm_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_md_format_movb4(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_md_format_movw4(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nd4_format_movb4(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nd4_format_movw4(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nmd_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d_format_branch(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d_format_mova(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d_format_movbg(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d_format_movwg(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d_format_movlg(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_d12_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nd8_format_movwi(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_nd8_format_movli(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_i_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);
extern void sh2e_insn_desc_dump_ni_format(sh2e_insn_desc_t const * desc, sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn, string_t * mnemonics, string_t * comments);

#endif // SUPERH_SH2E_DISASM_H_
