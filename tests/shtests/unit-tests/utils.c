#include "utils.h"

float32_bits_t signaling_nan = {
    .sign = 0,
    .exponent = 0xFF,
    .mantissa = 1 << 22,
    // If the bit 22 of the mantissa is set, it's a signaling NaN
};

float32_bits_t quiet_nan = {
    .sign = 0,
    .exponent = 0xFF,
    .mantissa = 0x1,
    // If the bit 22 of the mantissa is cleared, it's a quiet NaN
};

float32_bits_t positive_inf_num = {
    .sign = 0,
    .exponent = 0xFF,
    .mantissa = 0,
    // This represents the positive infinity number
};

float32_bits_t negative_inf_num = {
    .sign = 1,
    .exponent = 0xFF,
    .mantissa = 0,
    // This represents the negative infinity number
};

float32_bits_t positive_zero_num = {
    .sign = 0,
    .exponent = 0,
    .mantissa = 0,
    // This represents the positive zero number
};

float32_bits_t negative_zero_num = {
    .sign = 1,
    .exponent = 0,
    .mantissa = 0,
    // This represents the negative zero number
};

sh2e_insn_d_t insn_bt = {
    .ic = sh2e_insn_d_ic_bt,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_bts = {
    .ic = sh2e_insn_d_ic_bts,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_bf = {
    .ic = sh2e_insn_d_ic_bf,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_bfs = {
    .ic = sh2e_insn_d_ic_bfs,
    .d8 = 0x0,
};

sh2e_insn_nm_t insn_extsb = {
    .ic_h = sh2e_insn_nm_ic_h_extsb,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_extsb,
};

sh2e_insn_nm_t insn_extsw = {
    .ic_h = sh2e_insn_nm_ic_h_extsw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_extsw,
};

sh2e_insn_nm_t insn_extub = {
    .ic_h = sh2e_insn_nm_ic_h_extub,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_extub,
};

sh2e_insn_nm_t insn_extuw = {
    .ic_h = sh2e_insn_nm_ic_h_extuw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_extuw,
};

sh2e_insn_nd8_t insn_movwi = {
    .ic = sh2e_insn_nd8_ic_movwi,
    .rn = R0_REG,
    .d8 = 0x0,
};

sh2e_insn_nd8_t insn_movli = {
    .ic = sh2e_insn_nd8_ic_movli,
    .rn = R0_REG,
    .d8 = 0x0,
};

sh2e_insn_nm_t insn_movbl = {
    .ic_h = sh2e_insn_nm_ic_h_movbl,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbl,
};

sh2e_insn_nm_t insn_movwl = {
    .ic_h = sh2e_insn_nm_ic_h_movwl,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movwl,
};

sh2e_insn_nm_t insn_movll = {
    .ic_h = sh2e_insn_nm_ic_h_movll,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movll,
};

sh2e_insn_nm_t insn_movbs = {
    .ic_h = sh2e_insn_nm_ic_h_movbs,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbs,
};

sh2e_insn_nm_t insn_movws = {
    .ic_h = sh2e_insn_nm_ic_h_movws,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movws,
};

sh2e_insn_nm_t insn_movls = {
    .ic_h = sh2e_insn_nm_ic_h_movls,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movls,
};

sh2e_insn_nm_t insn_movbp = {
    .ic_h = sh2e_insn_nm_ic_h_movbp,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbp,
};

sh2e_insn_nm_t insn_movwp = {
    .ic_h = sh2e_insn_nm_ic_h_movwp,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movwp,
};

sh2e_insn_nm_t insn_movlp = {
    .ic_h = sh2e_insn_nm_ic_h_movlp,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movlp,
};

sh2e_insn_nm_t insn_movbm = {
    .ic_h = sh2e_insn_nm_ic_h_movbm,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbm,
};

sh2e_insn_nm_t insn_movwm = {
    .ic_h = sh2e_insn_nm_ic_h_movwm,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movwm,
};

sh2e_insn_nm_t insn_movlm = {
    .ic_h = sh2e_insn_nm_ic_h_movlm,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movlm,
};

sh2e_insn_nm_t insn_movbl0 = {
    .ic_h = sh2e_insn_nm_ic_h_movbl0,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbl0,
};

sh2e_insn_nm_t insn_movwl0 = {
    .ic_h = sh2e_insn_nm_ic_h_movwl0,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movwl0,
};

sh2e_insn_nm_t insn_movll0 = {
    .ic_h = sh2e_insn_nm_ic_h_movll0,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movll0,
};

sh2e_insn_nm_t insn_movbs0 = {
    .ic_h = sh2e_insn_nm_ic_h_movbs0,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movbs0,
};

sh2e_insn_nm_t insn_movws0 = {
    .ic_h = sh2e_insn_nm_ic_h_movws0,
    .rn = R1_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movws0,
};

sh2e_insn_nm_t insn_movls0 = {
    .ic_h = sh2e_insn_nm_ic_h_movls0,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_movls0,
};

sh2e_insn_md_t insn_movbl4 = {
    .ic = sh2e_insn_md_ic_movbl4,
    .rm = R0_REG,
    .d4 = 0x0,
};

sh2e_insn_md_t insn_movwl4 = {
    .ic = sh2e_insn_md_ic_movwl4,
    .rm = R0_REG,
    .d4 = 0x0,
};

sh2e_insn_nmd_t insn_movll4 = {
    .ic = sh2e_insn_nmd_ic_movll4,
    .rn = R0_REG,
    .d4 = 0x0,
    .rm = R1_REG,
};

sh2e_insn_nd4_t insn_movbs4 = {
    .ic = sh2e_insn_nd4_ic_movbs4,
    .rn = R0_REG,
    .d4 = 0x0,
};

sh2e_insn_nd4_t insn_movws4 = {
    .ic = sh2e_insn_nd4_ic_movws4,
    .rn = R0_REG,
    .d4 = 0x0,
};

sh2e_insn_nmd_t insn_movls4 = {
    .ic = sh2e_insn_nmd_ic_movls4,
    .rn = R0_REG,
    .d4 = 0x0,
    .rm = R1_REG,
};

sh2e_insn_d_t insn_movblg = {
    .ic = sh2e_insn_d_ic_movblg,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_movwlg = {
    .ic = sh2e_insn_d_ic_movwlg,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_movllg = {
    .ic = sh2e_insn_d_ic_movllg,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_movbsg = {
    .ic = sh2e_insn_d_ic_movbsg,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_movwsg = {
    .ic = sh2e_insn_d_ic_movwsg,
    .d8 = 0x0,
};

sh2e_insn_d_t insn_movlsg = {
    .ic = sh2e_insn_d_ic_movlsg,
    .d8 = 0x0,
};

sh2e_insn_nm_t insn_add = {
    .ic_h = sh2e_insn_nm_ic_h_add,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_add,
};

sh2e_insn_ni_t insn_addi = {
    .ic = sh2e_insn_ni_ic_addi,
    .rn = R0_REG,
    .i8 = 0x1,
};

sh2e_insn_nm_t insn_addc = {
    .ic_h = sh2e_insn_nm_ic_h_addc,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_addc,
};

sh2e_insn_nm_t insn_addv = {
    .ic_h = sh2e_insn_nm_ic_h_addv,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_addv,
};

sh2e_insn_nm_t insn_and = {
    .ic_h = sh2e_insn_nm_ic_h_and,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_and,
};

sh2e_insn_i_t insn_andi = {
    .ic = sh2e_insn_i_ic_andi,
    .i8 = 0x0,
};

sh2e_insn_i_t insn_andm = {
    .ic = sh2e_insn_i_ic_andm,
    .i8 = 0x0,
};

sh2e_insn_d12_t insn_bra = {
    .ic = sh2e_insn_d12_ic_bra,
    .d12 = 0x0,
};

sh2e_insn_m_t insn_braf = {
    .ic_h = sh2e_insn_m_ic_h_braf,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_braf,
};

sh2e_insn_d12_t insn_bsr = {
    .ic = sh2e_insn_d12_ic_bsr,
    .d12 = 0x0,
};

sh2e_insn_m_t insn_bsrf = {
    .ic_h = sh2e_insn_m_ic_h_bsrf,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_bsrf,
};

sh2e_insn_z_t insn_clrmac = {
    .ic = sh2e_insn_z_ic_clrmac,
};

sh2e_insn_z_t insn_clrt = {
    .ic = sh2e_insn_z_ic_clrt,
};

sh2e_insn_nm_t insn_cmpeq = {
    .ic_h = sh2e_insn_nm_ic_h_cmpeq,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmpeq,
};

sh2e_insn_nm_t insn_cmpge = {
    .ic_h = sh2e_insn_nm_ic_h_cmpge,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmpge,
};

sh2e_insn_nm_t insn_cmpgt = {
    .ic_h = sh2e_insn_nm_ic_h_cmpgt,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmpgt,
};

sh2e_insn_nm_t insn_cmphi = {
    .ic_h = sh2e_insn_nm_ic_h_cmphi,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmphi,
};

sh2e_insn_nm_t insn_cmphs = {
    .ic_h = sh2e_insn_nm_ic_h_cmphs,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmphs,
};

sh2e_insn_i_t insn_cmpim = {
    .ic = sh2e_insn_i_ic_cmpim,
    .i8 = 0x0,
};

sh2e_insn_n_t insn_cmppl = {
    .ic_h = sh2e_insn_n_ic_h_cmppl,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_cmppl
};

sh2e_insn_n_t insn_cmppz = {
    .ic_h = sh2e_insn_n_ic_h_cmppz,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_cmppz
};

sh2e_insn_nm_t insn_cmpstr = {
    .ic_h = sh2e_insn_nm_ic_h_cmpstr,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_cmpstr,
};

sh2e_insn_nm_t insn_div0s = {
    .ic_h = sh2e_insn_nm_ic_h_div0s,
    .rn = R1_REG,
    .rm = R0_REG,
    .ic_l = sh2e_insn_nm_ic_l_div0s,
};

sh2e_insn_z_t insn_div0u = {
    .ic = sh2e_insn_z_ic_div0u,
};

sh2e_insn_nm_t insn_div1 = {
    .ic_h = sh2e_insn_nm_ic_h_div1,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_div1,
};

sh2e_insn_nm_t insn_dmulsl = {
    .ic_h = sh2e_insn_nm_ic_h_dmulsl,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_dmulsl,
};

sh2e_insn_nm_t insn_dmulul = {
    .ic_h = sh2e_insn_nm_ic_h_dmulul,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_dmulul,
};

sh2e_insn_n_t insn_dt = {
    .ic_h = sh2e_insn_n_ic_h_dt,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_dt,
};

sh2e_insn_m_t insn_jmp = {
    .ic_h = sh2e_insn_m_ic_h_jmp,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_jmp,
};

sh2e_insn_m_t insn_jsr = {
    .ic_h = sh2e_insn_m_ic_h_jsr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_jsr,
};

sh2e_insn_m_t insn_ldc_sr = {
    .ic_h = sh2e_insn_m_ic_h_ldc_sr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldc_sr,
};

sh2e_insn_m_t insn_ldc_gbr = {
    .ic_h = sh2e_insn_m_ic_h_ldc_gbr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldc_gbr,
};

sh2e_insn_m_t insn_ldc_vbr = {
    .ic_h = sh2e_insn_m_ic_h_ldc_vbr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldc_vbr,
};

sh2e_insn_m_t insn_ldcl_sr = {
    .ic_h = sh2e_insn_m_ic_h_ldcl_sr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldcl_sr,
};

sh2e_insn_m_t insn_ldcl_gbr = {
    .ic_h = sh2e_insn_m_ic_h_ldcl_gbr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldcl_gbr,
};

sh2e_insn_m_t insn_ldcl_vbr = {
    .ic_h = sh2e_insn_m_ic_h_ldcl_vbr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldcl_vbr,
};

sh2e_insn_m_t insn_lds_mach = {
    .ic_h = sh2e_insn_m_ic_h_lds_mach,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_lds_mach,
};

sh2e_insn_m_t insn_lds_macl = {
    .ic_h = sh2e_insn_m_ic_h_lds_macl,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_lds_macl,
};

sh2e_insn_m_t insn_lds_pr = {
    .ic_h = sh2e_insn_m_ic_h_lds_pr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_lds_pr,
};

sh2e_insn_m_t insn_lds_fpscr = {
    .ic_h = sh2e_insn_m_ic_h_lds_fpscr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_lds_fpscr,
};

sh2e_insn_m_t insn_lds_fpul = {
    .ic_h = sh2e_insn_m_ic_h_lds_fpul,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_lds_fpul,
};

sh2e_insn_m_t insn_ldsl_mach = {
    .ic_h = sh2e_insn_m_ic_h_ldsl_mach,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldsl_mach,
};

sh2e_insn_m_t insn_ldsl_macl = {
    .ic_h = sh2e_insn_m_ic_h_ldsl_macl,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldsl_macl,
};

sh2e_insn_m_t insn_ldsl_pr = {
    .ic_h = sh2e_insn_m_ic_h_ldsl_pr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldsl_pr,
};

sh2e_insn_m_t insn_ldsl_fpscr = {
    .ic_h = sh2e_insn_m_ic_h_ldsl_fpscr,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldsl_fpscr,
};

sh2e_insn_m_t insn_ldsl_fpul = {
    .ic_h = sh2e_insn_m_ic_h_ldsl_fpul,
    .rm = R0_REG,
    .ic_l = sh2e_insn_m_ic_l_ldsl_fpul,
};

sh2e_insn_nm_t insn_macl = {
    .ic_h = sh2e_insn_nm_ic_h_macl,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_macl,
};

sh2e_insn_nm_t insn_macw = {
    .ic_h = sh2e_insn_nm_ic_h_macw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_macw,
};

sh2e_insn_nm_t insn_mov = {
    .ic_h = sh2e_insn_nm_ic_h_mov,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_mov,
};

sh2e_insn_ni_t insn_movi = {
    .ic = sh2e_insn_ni_ic_movi,
    .rn = R0_REG,
    .i8 = 0x0,
};

sh2e_insn_d_t insn_mova = {
    .ic = sh2e_insn_d_ic_mova,
    .d8 = 0x0,
};

sh2e_insn_n_t insn_movt = {
    .ic_h = sh2e_insn_n_ic_h_movt,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_movt,
};

sh2e_insn_nm_t insn_mull = {
    .ic_h = sh2e_insn_nm_ic_h_mull,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_mull,
};

sh2e_insn_nm_t insn_mulsw = {
    .ic_h = sh2e_insn_nm_ic_h_mulsw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_mulsw,
};

sh2e_insn_nm_t insn_muluw = {
    .ic_h = sh2e_insn_nm_ic_h_muluw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_muluw,
};

sh2e_insn_nm_t insn_neg = {
    .ic_h = sh2e_insn_nm_ic_h_neg,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_neg,
};

sh2e_insn_nm_t insn_negc = {
    .ic_h = sh2e_insn_nm_ic_h_negc,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_negc,
};

sh2e_insn_z_t insn_nop = {
    .ic = sh2e_insn_z_ic_nop,
};

sh2e_insn_nm_t insn_not = {
    .ic_h = sh2e_insn_nm_ic_h_not,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_not,
};

sh2e_insn_nm_t insn_or = {
    .ic_h = sh2e_insn_nm_ic_h_or,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_or,
};

sh2e_insn_i_t insn_ori = {
    .ic = sh2e_insn_i_ic_ori,
    .i8 = 0x0,
};

sh2e_insn_i_t insn_orm = {
    .ic = sh2e_insn_i_ic_orm,
    .i8 = 0x0,
};

sh2e_insn_z_t insn_rte = {
    .ic = sh2e_insn_z_ic_rte,
};

sh2e_insn_z_t insn_rts = {
    .ic = sh2e_insn_z_ic_rts,
};

sh2e_insn_n_t insn_rotcl = {
    .ic_h = sh2e_insn_n_ic_h_rotcl,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_rotcl,
};

sh2e_insn_n_t insn_rotcr = {
    .ic_h = sh2e_insn_n_ic_h_rotcr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_rotcr,
};

sh2e_insn_n_t insn_rotl = {
    .ic_h = sh2e_insn_n_ic_h_rotl,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_rotl,
};

sh2e_insn_n_t insn_rotr = {
    .ic_h = sh2e_insn_n_ic_h_rotr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_rotr,
};

sh2e_insn_z_t insn_sett = {
    .ic = sh2e_insn_z_ic_sett,
};

sh2e_insn_n_t insn_shal = {
    .ic_h = sh2e_insn_n_ic_h_shal,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shal,
};

sh2e_insn_n_t insn_shar = {
    .ic_h = sh2e_insn_n_ic_h_shar,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shar,
};

sh2e_insn_n_t insn_shll = {
    .ic_h = sh2e_insn_n_ic_h_shll,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shll,
};

sh2e_insn_n_t insn_shll2 = {
    .ic_h = sh2e_insn_n_ic_h_shll2,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shll2,
};

sh2e_insn_n_t insn_shll8 = {
    .ic_h = sh2e_insn_n_ic_h_shll8,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shll8,
};

sh2e_insn_n_t insn_shll16 = {
    .ic_h = sh2e_insn_n_ic_h_shll16,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shll16,
};

sh2e_insn_n_t insn_shlr = {
    .ic_h = sh2e_insn_n_ic_h_shlr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shlr,
};

sh2e_insn_n_t insn_shlr2 = {
    .ic_h = sh2e_insn_n_ic_h_shlr2,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shlr2,
};

sh2e_insn_n_t insn_shlr8 = {
    .ic_h = sh2e_insn_n_ic_h_shlr8,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shlr8,
};

sh2e_insn_n_t insn_shlr16 = {
    .ic_h = sh2e_insn_n_ic_h_shlr16,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_shlr16,
};

sh2e_insn_z_t insn_sleep = {
    .ic = sh2e_insn_z_ic_sleep,
};

sh2e_insn_n_t insn_stc_sr = {
    .ic_h = sh2e_insn_n_ic_h_stc_sr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stc_sr,
};

sh2e_insn_n_t insn_stc_gbr = {
    .ic_h = sh2e_insn_n_ic_h_stc_gbr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stc_gbr,
};

sh2e_insn_n_t insn_stc_vbr = {
    .ic_h = sh2e_insn_n_ic_h_stc_vbr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stc_vbr,
};

sh2e_insn_n_t insn_stcm_sr = {
    .ic_h = sh2e_insn_n_ic_h_stcm_sr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stcm_sr,
};

sh2e_insn_n_t insn_stcm_gbr = {
    .ic_h = sh2e_insn_n_ic_h_stcm_gbr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stcm_gbr,
};

sh2e_insn_n_t insn_stcm_vbr = {
    .ic_h = sh2e_insn_n_ic_h_stcm_vbr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stcm_vbr,
};

sh2e_insn_n_t insn_sts_fpul = {
    .ic_h = sh2e_insn_n_ic_h_sts_fpul,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_sts_fpul,
};

sh2e_insn_n_t insn_sts_fpscr = {
    .ic_h = sh2e_insn_n_ic_h_sts_fpscr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_sts_fpscr,
};

sh2e_insn_n_t insn_sts_mach = {
    .ic_h = sh2e_insn_n_ic_h_sts_mach,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_sts_mach,
};

sh2e_insn_n_t insn_sts_macl = {
    .ic_h = sh2e_insn_n_ic_h_sts_macl,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_sts_macl,
};

sh2e_insn_n_t insn_sts_pr = {
    .ic_h = sh2e_insn_n_ic_h_sts_pr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_sts_pr,
};

sh2e_insn_n_t insn_stsm_fpul = {
    .ic_h = sh2e_insn_n_ic_h_stsm_fpul,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stsm_fpul,
};

sh2e_insn_n_t insn_stsm_fpscr = {
    .ic_h = sh2e_insn_n_ic_h_stsm_fpscr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stsm_fpscr,
};

sh2e_insn_n_t insn_stsm_mach = {
    .ic_h = sh2e_insn_n_ic_h_stsm_mach,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stsm_mach,
};

sh2e_insn_n_t insn_stsm_macl = {
    .ic_h = sh2e_insn_n_ic_h_stsm_macl,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stsm_macl,
};

sh2e_insn_n_t insn_stsm_pr = {
    .ic_h = sh2e_insn_n_ic_h_stsm_pr,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_stsm_pr,
};

sh2e_insn_nm_t insn_sub = {
    .ic_h = sh2e_insn_nm_ic_h_sub,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_sub,
};

sh2e_insn_nm_t insn_subc = {
    .ic_h = sh2e_insn_nm_ic_h_subc,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_subc,
};

sh2e_insn_nm_t insn_subv = {
    .ic_h = sh2e_insn_nm_ic_h_subv,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_subv,
};

sh2e_insn_nm_t insn_swapb = {
    .ic_h = sh2e_insn_nm_ic_h_swapb,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_swapb,
};

sh2e_insn_nm_t insn_swapw = {
    .ic_h = sh2e_insn_nm_ic_h_swapw,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_swapw,
};

sh2e_insn_n_t insn_tas = {
    .ic_h = sh2e_insn_n_ic_h_tas,
    .rn = R0_REG,
    .ic_l = sh2e_insn_n_ic_l_tas,
};

sh2e_insn_i_t insn_trapa = {
    .ic = sh2e_insn_i_ic_trapa,
    .i8 = 0x0,
};

sh2e_insn_nm_t insn_tst = {
    .ic_h = sh2e_insn_nm_ic_h_tst,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_tst,
};

sh2e_insn_i_t insn_tsti = {
    .ic = sh2e_insn_i_ic_tsti,
    .i8 = 0x0,
};

sh2e_insn_i_t insn_tstm = {
    .ic = sh2e_insn_i_ic_tstm,
    .i8 = 0x0,
};

sh2e_insn_nm_t insn_xor = {
    .ic_h = sh2e_insn_nm_ic_h_xor,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_xor,
};

sh2e_insn_i_t insn_xori = {
    .ic = sh2e_insn_i_ic_xori,
    .i8 = 0x0,
};

sh2e_insn_i_t insn_xorm = {
    .ic = sh2e_insn_i_ic_xorm,
    .i8 = 0x0,
};

sh2e_insn_nm_t insn_xtrct = {
    .ic_h = sh2e_insn_nm_ic_h_xtrct,
    .rn = R0_REG,
    .rm = R1_REG,
    .ic_l = sh2e_insn_nm_ic_l_xtrct,
};

sh2e_insn_n_t insn_fabs = {
    .ic_h = sh2e_insn_n_ic_h_fabs,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_fabs,
};

sh2e_insn_nm_t insn_fadd = {
    .ic_h = sh2e_insn_nm_ic_h_fadd,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fadd,
};

sh2e_insn_nm_t insn_fcmpeq = {
    .ic_h = sh2e_insn_nm_ic_h_fcmpeq,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fcmpeq,
};

sh2e_insn_nm_t insn_fcmpgt = {
    .ic_h = sh2e_insn_nm_ic_h_fcmpgt,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fcmpgt,
};

sh2e_insn_nm_t insn_fdiv = {
    .ic_h = sh2e_insn_nm_ic_h_fdiv,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fdiv,
};

sh2e_insn_n_t insn_fldi0 = {
    .ic_h = sh2e_insn_n_ic_h_fldi0,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_fldi0,
};

sh2e_insn_n_t insn_fldi1 = {
    .ic_h = sh2e_insn_n_ic_h_fldi1,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_fldi1,
};

sh2e_insn_m_t insn_flds = {
    .ic_h = sh2e_insn_m_ic_h_flds,
    .rm = FR0_REG,
    .ic_l = sh2e_insn_m_ic_l_flds,
};

sh2e_insn_n_t insn_float = {
    .ic_h = sh2e_insn_n_ic_h_float,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_float,
};

sh2e_insn_nm_t insn_fmac = {
    .ic_h = sh2e_insn_nm_ic_h_fmac,
    // FR2 here in order to use 3 registers instead of 2 (FMAC uses FR0,FRm and FRn)
    .rn = FR2_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmac,
};

sh2e_insn_nm_t insn_fmov = {
    .ic_h = sh2e_insn_nm_ic_h_fmov,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmov,
};

sh2e_insn_nm_t insn_fmovl = {
    .ic_h = sh2e_insn_nm_ic_h_fmovl,
    .rn = FR0_REG,
    .rm = R0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovl,
};

sh2e_insn_nm_t insn_fmovli = {
    .ic_h = sh2e_insn_nm_ic_h_fmovli,
    .rn = FR0_REG,
    .rm = R0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovli,
};

sh2e_insn_nm_t insn_fmovlr = {
    .ic_h = sh2e_insn_nm_ic_h_fmovlr,
    .rn = FR0_REG,
    .rm = R0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovlr,
};

sh2e_insn_nm_t insn_fmovs = {
    .ic_h = sh2e_insn_nm_ic_h_fmovs,
    .rn = R0_REG,
    .rm = FR0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovs,
};

sh2e_insn_nm_t insn_fmovsi = {
    .ic_h = sh2e_insn_nm_ic_h_fmovsi,
    .rn = R1_REG,
    .rm = FR0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovsi,
};

sh2e_insn_nm_t insn_fmovss = {
    .ic_h = sh2e_insn_nm_ic_h_fmovss,
    .rn = R0_REG,
    .rm = FR0_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmovss,
};

sh2e_insn_nm_t insn_fmul = {
    .ic_h = sh2e_insn_nm_ic_h_fmul,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fmul,
};

sh2e_insn_n_t insn_fneg = {
    .ic_h = sh2e_insn_n_ic_h_fneg,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_fneg,
};

sh2e_insn_n_t insn_fsts = {
    .ic_h = sh2e_insn_n_ic_h_fsts,
    .rn = FR0_REG,
    .ic_l = sh2e_insn_n_ic_l_fsts,
};

sh2e_insn_nm_t insn_fsub = {
    .ic_h = sh2e_insn_nm_ic_h_fsub,
    .rn = FR0_REG,
    .rm = FR1_REG,
    .ic_l = sh2e_insn_nm_ic_l_fsub,
};

sh2e_insn_m_t insn_ftrc = {
    .ic_h = sh2e_insn_m_ic_h_ftrc,
    .rm = FR0_REG,
    .ic_l = sh2e_insn_m_ic_l_ftrc,
};

sh2e_insn_z_t insn_illegal = {
    .ic = 0xFFFF,
};

sh2e_insn_z_t insn_halt = {
    .ic = sh2e_insn_z_ic_halt,
};

sh2e_insn_z_t insn_cpu_reg_dump = {
    .ic = sh2e_insn_z_ic_cpu_reg_dump,
};

sh2e_insn_z_t insn_fpu_reg_dump = {
    .ic = sh2e_insn_z_ic_fpu_reg_dump,
};
