#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"

PCUT_INIT

PCUT_TEST_SUITE(Instruction);

PCUT_TEST(s_imm_positive){
    rv_instr_t instr;
    instr.s.imm4_0 = 1;
    instr.s.imm11_5 = 1;

    PCUT_ASSERT_INT_EQUALS(33, RV_S_IMM(instr));
}

PCUT_EXPORT(Instruction);