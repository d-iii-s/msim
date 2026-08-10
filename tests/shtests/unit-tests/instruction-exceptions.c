#include <stdint.h>
#include <string.h>
#include <pcut/pcut.h>

#include "../../../src/device/cpu/superh_sh2e/cpu.h"
#include "../../../src/device/cpu/superh_sh2e/exec.h"
#include "../../../src/device/cpu/superh_sh2e/insn.h"
#include "utils.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_exceptions);

sh2e_cpu_t cpu1;

PCUT_TEST_BEFORE
{
    sh2e_cpu_init(&cpu1, 0);
}

PCUT_TEST(bt_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bt(&cpu1, insn_bt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bt_no_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_NONE;

    sh2e_exception_t ex = sh2e_insn_exec_bt(&cpu1, insn_bt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bts_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bts(&cpu1, insn_bts);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bts_no_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_NONE;

    sh2e_exception_t ex = sh2e_insn_exec_bts(&cpu1, insn_bts);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bf_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bf(&cpu1, insn_bf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bf_no_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_NONE;

    sh2e_exception_t ex = sh2e_insn_exec_bf(&cpu1, insn_bf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bfs_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bfs(&cpu1, insn_bfs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bfs_no_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_NONE;

    sh2e_exception_t ex = sh2e_insn_exec_bfs(&cpu1, insn_bfs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(extsb_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_extsb(&cpu1, insn_extsb);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(extsw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_extsw(&cpu1, insn_extsw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(extub_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_extub(&cpu1, insn_extub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(extuw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_extuw(&cpu1, insn_extuw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwi(&cpu1, insn_movwi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movli_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movli(&cpu1, insn_movli);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movbl_no_ex)
{

    sh2e_exception_t ex = sh2e_insn_exec_movbl(&cpu1, insn_movbl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwl(&cpu1, insn_movwl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movwl.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwl(&cpu1, insn_movwl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movll_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movll(&cpu1, insn_movll);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movll_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movll.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movll(&cpu1, insn_movll);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbs_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbs(&cpu1, insn_movbs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movws(&cpu1, insn_movws);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movws.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movws(&cpu1, insn_movws);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movls_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movls(&cpu1, insn_movls);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movls_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movls.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movls(&cpu1, insn_movls);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbp_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbp(&cpu1, insn_movbp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwp_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwp(&cpu1, insn_movwp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwp_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movwp.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwp(&cpu1, insn_movwp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movlp_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movlp(&cpu1, insn_movlp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movlp_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movlp.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movlp(&cpu1, insn_movlp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbm_no_ex)
{
    cpu1.cpu_regs.general[insn_movbm.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movbm(&cpu1, insn_movbm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwm_no_ex)
{
    cpu1.cpu_regs.general[insn_movwm.rn] = 0x02;

    sh2e_exception_t ex = sh2e_insn_exec_movwm(&cpu1, insn_movwm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwm_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movwm.rn] = 0x03;

    sh2e_exception_t ex = sh2e_insn_exec_movwm(&cpu1, insn_movwm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movlm_no_ex)
{
    cpu1.cpu_regs.general[insn_movlm.rn] = 0x04;

    sh2e_exception_t ex = sh2e_insn_exec_movlm(&cpu1, insn_movlm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movlm_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movlm.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_movlm(&cpu1, insn_movlm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbl0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbl0(&cpu1, insn_movbl0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwl0(&cpu1, insn_movwl0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl0_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movwl0.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwl0(&cpu1, insn_movwl0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movll0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movll0(&cpu1, insn_movll0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movll0_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movll0.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movll0(&cpu1, insn_movll0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbs0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbs0(&cpu1, insn_movbs0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movws0(&cpu1, insn_movws0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws0_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movws0.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movws0(&cpu1, insn_movws0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movls0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movls0(&cpu1, insn_movls0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movls0_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movls0.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movls0(&cpu1, insn_movls0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbl4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbl4(&cpu1, insn_movbl4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwl4(&cpu1, insn_movwl4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwl4_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movwl4.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwl4(&cpu1, insn_movwl4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movll4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movll4(&cpu1, insn_movll4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movll4_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movll4.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movll4(&cpu1, insn_movll4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbs4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbs4(&cpu1, insn_movbs4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movws4(&cpu1, insn_movws4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movws4_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movws4.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movws4(&cpu1, insn_movws4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movls4_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movls4(&cpu1, insn_movls4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movls4_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_movls4.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movls4(&cpu1, insn_movls4);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movblg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movblg(&cpu1, insn_movblg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwlg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwlg(&cpu1, insn_movwlg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwlg_cpu_address_ex)
{
    cpu1.cpu_regs.gbr = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwlg(&cpu1, insn_movwlg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movllg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movllg(&cpu1, insn_movllg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movllg_cpu_address_ex)
{
    cpu1.cpu_regs.gbr = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movllg(&cpu1, insn_movllg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movbsg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movbsg(&cpu1, insn_movbsg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwsg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movwsg(&cpu1, insn_movwsg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movwsg_cpu_address_ex)
{
    cpu1.cpu_regs.gbr = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movwsg(&cpu1, insn_movwsg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(movlsg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movlsg(&cpu1, insn_movlsg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movlsg_cpu_address_ex)
{
    cpu1.cpu_regs.gbr = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_movlsg(&cpu1, insn_movlsg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(add_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_add(&cpu1, insn_add);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(addi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_addi(&cpu1, insn_addi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(addc_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_addc(&cpu1, insn_addc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(addv_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_addv(&cpu1, insn_addv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(and_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_and(&cpu1, insn_and);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(andi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_andi(&cpu1, insn_andi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(andm_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_andm(&cpu1, insn_andm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bra_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_bra(&cpu1, insn_bra);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bra_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bra(&cpu1, insn_bra);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(braf_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_braf(&cpu1, insn_braf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(braf_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_braf(&cpu1, insn_braf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bsr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_bsr(&cpu1, insn_bsr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bsr_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bsr(&cpu1, insn_bsr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(bsrf_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_bsrf(&cpu1, insn_bsrf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(bsrf_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_bsrf(&cpu1, insn_bsrf);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(clrmac_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_clrmac(&cpu1, insn_clrmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(clrt_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_clrt(&cpu1, insn_clrt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmpgeq_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmpeq(&cpu1, insn_cmpeq);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmpge_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmpge(&cpu1, insn_cmpge);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmpgt_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmpgt(&cpu1, insn_cmpgt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmphi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmphi(&cpu1, insn_cmphi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmphs_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmphs(&cpu1, insn_cmphs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmpim_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmpim(&cpu1, insn_cmpim);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmppl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmppl(&cpu1, insn_cmppl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmppz_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmppz(&cpu1, insn_cmppz);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(cmpstr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_cmpstr(&cpu1, insn_cmpstr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(div0s_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_div0s(&cpu1, insn_div0s);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(div0u_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_div0u(&cpu1, insn_div0u);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(div1_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_div1(&cpu1, insn_div1);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(dmulsl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_dmulsl(&cpu1, insn_dmulsl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(dmulul_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_dmulul(&cpu1, insn_dmulul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(dt_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_dt(&cpu1, insn_dt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(jmp_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_jmp(&cpu1, insn_jmp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(jmp_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_jmp(&cpu1, insn_jmp);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(jsr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_jsr(&cpu1, insn_jsr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(jsr_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_jsr(&cpu1, insn_jsr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(ldc_sr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldc_sr(&cpu1, insn_ldc_sr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldc_gbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldc_gbr(&cpu1, insn_ldc_gbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldc_vbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldc_vbr(&cpu1, insn_ldc_vbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldcl_sr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldcl_sr(&cpu1, insn_ldcl_sr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldcl_sr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldcl_sr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldcl_sr(&cpu1, insn_ldcl_sr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldcl_gbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldcl_gbr(&cpu1, insn_ldcl_gbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldcl_gbr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldcl_gbr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldcl_gbr(&cpu1, insn_ldcl_gbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldcl_vbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldcl_vbr(&cpu1, insn_ldcl_vbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldcl_vbr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldcl_vbr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldcl_vbr(&cpu1, insn_ldcl_vbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(lds_mach_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_lds_mach(&cpu1, insn_lds_mach);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(lds_macl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_lds_macl(&cpu1, insn_lds_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(lds_pr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_lds_pr(&cpu1, insn_lds_pr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(lds_fpscr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_lds_fpscr(&cpu1, insn_lds_fpscr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(lds_fpul_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_lds_fpul(&cpu1, insn_lds_fpul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_mach_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldsl_mach(&cpu1, insn_ldsl_mach);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_mach_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldsl_mach.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldsl_mach(&cpu1, insn_ldsl_mach);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldsl_macl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldsl_macl(&cpu1, insn_ldsl_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_macl_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldsl_macl.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldsl_macl(&cpu1, insn_ldsl_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldsl_pr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldsl_pr(&cpu1, insn_ldsl_pr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_pr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldsl_pr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldsl_pr(&cpu1, insn_ldsl_pr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldsl_fpscr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldsl_fpscr(&cpu1, insn_ldsl_fpscr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_fpscr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldsl_fpscr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldsl_fpscr(&cpu1, insn_ldsl_fpscr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(ldsl_fpul_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ldsl_fpul(&cpu1, insn_ldsl_fpul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ldsl_fpul_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_ldsl_fpul.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_ldsl_fpul(&cpu1, insn_ldsl_fpul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(macl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_macl(&cpu1, insn_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(macl_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_macl.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_macl(&cpu1, insn_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(macw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_macw(&cpu1, insn_macw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(macw_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_macw.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_macw(&cpu1, insn_macw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(mov_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_mov(&cpu1, insn_mov);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movi(&cpu1, insn_movi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(mova_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_mova(&cpu1, insn_mova);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(movt_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_movt(&cpu1, insn_movt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(mull_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_mull(&cpu1, insn_mull);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(mulsw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_mulsw(&cpu1, insn_mulsw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(muluw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_muluw(&cpu1, insn_muluw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(neg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_neg(&cpu1, insn_neg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(negc_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_negc(&cpu1, insn_negc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(nop_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_nop(&cpu1, insn_nop);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(not_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_not(&cpu1, insn_not);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(or_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_or(&cpu1, insn_or);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ori_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ori(&cpu1, insn_ori);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(orm_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_orm(&cpu1, insn_orm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rte_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rte(&cpu1, insn_rte);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rte_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_rte(&cpu1, insn_rte);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(rte_cpu_address_ex)
{
    cpu1.cpu_regs.sp = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_rte(&cpu1, insn_rte);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(rts_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rts(&cpu1, insn_rts);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rts_illegal_slot_insn_ex)
{
    cpu1.br_state = SH2E_BRANCH_STATE_DELAY;

    sh2e_exception_t ex = sh2e_insn_exec_rts(&cpu1, insn_rts);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION, ex);
}

PCUT_TEST(rotcl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rotcl(&cpu1, insn_rotcl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rotcr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rotcr(&cpu1, insn_rotcr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rotl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rotl(&cpu1, insn_rotl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(rotr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_rotr(&cpu1, insn_rotr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sett_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sett(&cpu1, insn_sett);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shal_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shal(&cpu1, insn_shal);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shar_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shar(&cpu1, insn_shar);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shll_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shll(&cpu1, insn_shll);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shll2_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shll2(&cpu1, insn_shll2);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shll8_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shll8(&cpu1, insn_shll8);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shll16_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shll16(&cpu1, insn_shll16);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shlr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shlr(&cpu1, insn_shlr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shlr2_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shlr2(&cpu1, insn_shlr2);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shlr8_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shlr8(&cpu1, insn_shlr8);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(shlr16_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_shlr16(&cpu1, insn_shlr16);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sleep_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sleep(&cpu1, insn_sleep);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(stc_sr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_stc_cpu(&cpu1, insn_stc_sr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(stc_gbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_stc_cpu(&cpu1, insn_stc_gbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(stc_vbr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_stc_cpu(&cpu1, insn_stc_vbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

// PCUT_TEST(stcm_sr_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_sr);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stcm_sr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stcm_sr.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_sr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stcm_gbr_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_gbr);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stcm_gbr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stcm_gbr.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_gbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stcm_vbr_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_vbr);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stcm_vbr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stcm_vbr.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stcm_cpu(&cpu1, insn_stcm_vbr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(sts_fpul_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sts_fpu(&cpu1, insn_sts_fpul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sts_fpscr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sts_fpu(&cpu1, insn_sts_fpscr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sts_mach_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sts_cpu(&cpu1, insn_sts_mach);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sts_macl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sts_cpu(&cpu1, insn_sts_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(sts_pr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sts_cpu(&cpu1, insn_sts_pr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

// PCUT_TEST(stsm_fpul_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stsm_fpu(&cpu1, insn_stsm_fpul);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stsm_fpul_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stsm_fpul.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stsm_fpu(&cpu1, insn_stsm_fpul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stsm_fpscr_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stsm_fpu(&cpu1, insn_stsm_fpscr);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stsm_fpscr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stsm_fpscr.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stsm_fpu(&cpu1, insn_stsm_fpscr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stsm_mach_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_mach);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stsm_mach_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stsm_mach.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_mach);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stsm_macl_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_macl);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stsm_macl_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stsm_macl.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_macl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

// PCUT_TEST(stsm_pr_no_ex)
// {
//     sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_pr);

//     PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
// }

PCUT_TEST(stsm_pr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_stsm_pr.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_stsm_cpu(&cpu1, insn_stsm_pr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(sub_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_sub(&cpu1, insn_sub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(subc_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_subc(&cpu1, insn_subc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(subv_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_subv(&cpu1, insn_subv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(swapb_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_swapb(&cpu1, insn_swapb);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(swapw_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_swapw(&cpu1, insn_swapw);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(tas_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_tas(&cpu1, insn_tas);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(trapa_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_trapa(&cpu1, insn_trapa);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(trapa_cpu_address_ex)
{
    cpu1.cpu_regs.sp = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_trapa(&cpu1, insn_trapa);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(tst_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_tst(&cpu1, insn_tst);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(tsti_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_tsti(&cpu1, insn_tsti);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(tstm_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_tstm(&cpu1, insn_tstm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(xor_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_xor(&cpu1, insn_xor);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(xori_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_xori(&cpu1, insn_xori);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(xorm_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_xorm(&cpu1, insn_xorm);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(xtrct_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_xtrct(&cpu1, insn_xtrct);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fabs_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fabs(&cpu1, insn_fabs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fabs_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fabs.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fabs(&cpu1, insn_fabs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fadd_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fadd(&cpu1, insn_fadd);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fadd_ninf_pinf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fadd.rn] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fadd.rm] = negative_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fadd(&cpu1, insn_fadd);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fadd_pinf_ninf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fadd.rn] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fadd.rm] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fadd(&cpu1, insn_fadd);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fadd_snan_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fadd.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fadd(&cpu1, insn_fadd);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fcmpeq_no_ex)
{
    // Added here quiet NaN to test that no exception is raised
    cpu1.fpu_regs.general[insn_fcmpeq.rn] = quiet_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fcmpeq(&cpu1, insn_fcmpeq);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fcmpeq_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fcmpeq.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fcmpeq(&cpu1, insn_fcmpeq);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fcmpgt_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fcmpgt(&cpu1, insn_fcmpgt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fcmpgt_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fcmpgt.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fcmpgt(&cpu1, insn_fcmpgt);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fdiv_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fdiv(&cpu1, insn_fdiv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fdiv_both_zero_ex)
{
    cpu1.fpu_regs.general[insn_fdiv.rn] = positive_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fdiv.rm] = positive_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fdiv(&cpu1, insn_fdiv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fdiv_both_inf_ex)
{
    cpu1.fpu_regs.general[insn_fdiv.rn] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fdiv.rm] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fdiv(&cpu1, insn_fdiv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fdiv_div_by_zero_ex)
{
    cpu1.fpu_regs.general[insn_fdiv.rn] = 1.0f;
    cpu1.fpu_regs.general[insn_fdiv.rm] = positive_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ez = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fdiv(&cpu1, insn_fdiv);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fldi0_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fldi0(&cpu1, insn_fldi0);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fldi1_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fldi1(&cpu1, insn_fldi1);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(flds_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_flds(&cpu1, insn_flds);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(float_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_float(&cpu1, insn_float);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmac_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmac_zero_inf_norm_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = 1.0f;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_inf_zero_norm_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = 1.0f;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_zero_inf_pzero_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = positive_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_inf_zero_pzero_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = positive_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_zero_inf_nzero_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = negative_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_inf_zero_nzero_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = negative_zero_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_zero_pinf_ninf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = negative_inf_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_nnorm_pinf_pinf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = -1.0f;
    cpu1.fpu_regs.general[insn_fmac.rm] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_inf_zero_qnan_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = negative_zero_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rn] = quiet_nan.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_ninf_pnorm_pinf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = 1.0f;
    cpu1.fpu_regs.general[insn_fmac.rn] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmac_pinf_pnorm_ninf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[0] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fmac.rm] = 1.0f;
    cpu1.fpu_regs.general[insn_fmac.rn] = negative_inf_num.fvalue;

    // Have to enable this bit to raise FPU exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmac(&cpu1, insn_fmac);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmov_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmov(&cpu1, insn_fmov);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovl_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmovl(&cpu1, insn_fmovl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovl_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovl.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_fmovl(&cpu1, insn_fmovl);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmovli_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmovli(&cpu1, insn_fmovli);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovli_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovli.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_fmovli(&cpu1, insn_fmovli);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmovlr_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmovlr(&cpu1, insn_fmovlr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovlr_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovlr.rm] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_fmovlr(&cpu1, insn_fmovlr);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmovs_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmovs(&cpu1, insn_fmovs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovs_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovs.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_fmovs(&cpu1, insn_fmovs);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmovsi_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmovsi(&cpu1, insn_fmovsi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovsi_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovsi.rn] = 0x01;

    sh2e_exception_t ex = sh2e_insn_exec_fmovsi(&cpu1, insn_fmovsi);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmovss_no_ex)
{
    cpu1.cpu_regs.general[insn_fmovss.rn] = 0x4;

    sh2e_exception_t ex = sh2e_insn_exec_fmovss(&cpu1, insn_fmovss);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmovss_cpu_address_ex)
{
    cpu1.cpu_regs.general[insn_fmovss.rn] = 0x05;

    sh2e_exception_t ex = sh2e_insn_exec_fmovss(&cpu1, insn_fmovss);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_CPU_ADDRESS_ERROR, ex);
}

PCUT_TEST(fmul_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fmul(&cpu1, insn_fmul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fmul_snan_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fmul.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmul(&cpu1, insn_fmul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fmul_inf_zero_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fmul.rn] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fmul(&cpu1, insn_fmul);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fneg_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fneg(&cpu1, insn_fneg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fneg_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fneg.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fneg(&cpu1, insn_fneg);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fsts_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fsts(&cpu1, insn_fsts);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

// NOTE: there are many cases for FSUB exceptions, only a few are tested here

PCUT_TEST(fsub_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_fsub(&cpu1, insn_fsub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fsub_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_fsub.rn] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fsub(&cpu1, insn_fsub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fsub_pos_inf_num_and_zero_no_ex)
{
    cpu1.fpu_regs.general[insn_fsub.rn] = positive_inf_num.fvalue;

    sh2e_exception_t ex = sh2e_insn_exec_fsub(&cpu1, insn_fsub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(fsub_both_pos_inf_num_fpu_operation_ex)
{
    // Both operands being positive infinity numbers should raise FPU operation exception
    cpu1.fpu_regs.general[insn_fsub.rn] = positive_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fsub.rm] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fsub(&cpu1, insn_fsub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(fsub_both_neg_inf_num_fpu_operation_ex)
{
    // Both operands being negative infinity numbers should raise FPU operation exception
    cpu1.fpu_regs.general[insn_fsub.rn] = negative_inf_num.fvalue;
    cpu1.fpu_regs.general[insn_fsub.rm] = negative_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_fsub(&cpu1, insn_fsub);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(ftrc_no_ex)
{
    sh2e_exception_t ex = sh2e_insn_exec_ftrc(&cpu1, insn_ftrc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_NONE, ex);
}

PCUT_TEST(ftrc_fpu_operation_ex_nan)
{
    cpu1.fpu_regs.general[insn_ftrc.rm] = signaling_nan.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_ftrc(&cpu1, insn_ftrc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_TEST(ftrc_pos_inf_fpu_operation_ex)
{
    cpu1.fpu_regs.general[insn_ftrc.rm] = positive_inf_num.fvalue;

    // Have to enable this bit to raise FPU operation exception
    cpu1.fpu_regs.fpscr.ev = 0x1;

    sh2e_exception_t ex = sh2e_insn_exec_ftrc(&cpu1, insn_ftrc);

    PCUT_ASSERT_INT_EQUALS(SH2E_EXCEPTION_FPU_OPERATION, ex);
}

PCUT_EXPORT(instruction_exceptions);
