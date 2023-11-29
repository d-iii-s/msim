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

#ifndef SUPERH_SH2E_DECODE_INC_
#define SUPERH_SH2E_DECODE_INC_


// Instruction decoding functions

extern sh2e_insn_desc_t const * sh2e_insn_decode_z_format(sh2e_insn_z_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_n_format(sh2e_insn_n_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_n_format_fpu(sh2e_insn_n_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_m_format(sh2e_insn_m_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_m_format_fpu(sh2e_insn_m_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_nm_format(sh2e_insn_nm_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_nm_format_fpu(sh2e_insn_nm_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_md_format(sh2e_insn_md_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_nd4_format(sh2e_insn_nd4_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_nmd_format(sh2e_insn_nmd_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_d_format(sh2e_insn_d_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_d12_format(sh2e_insn_d12_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_nd8_format(sh2e_insn_nd8_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_i_format(sh2e_insn_i_t insn);
extern sh2e_insn_desc_t const * sh2e_insn_decode_ni_format(sh2e_insn_ni_t insn);

#endif // SUPERH_SH2E_DECODE_INC_
