#include <stdint.h>
#include <pcut/pcut.h>

#include "common.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_immediates);

PCUT_TEST(s_imm_positive)
{
    rv_instr_t instr;
    instr.s.imm4_0 = 1;
    instr.s.imm11_5 = 1;

    PCUT_ASSERT_INT_EQUALS(33, RV_S_IMM(instr));
}

PCUT_TEST(s_imm_negative)
{
    rv_instr_t instr;
    instr.s.imm4_0 = -1;
    instr.s.imm11_5 = -1;

    // the 12 bits should be sign extended
    PCUT_ASSERT_INT_EQUALS(-1, (int32_t) RV_S_IMM(instr));
}

PCUT_TEST(j_imm_positive)
{
    rv_instr_t instr;
    instr.j.imm10_1 = 1;
    instr.j.imm11 = 1;
    instr.j.imm19_12 = 1;
    instr.j.imm20 = 0;

    //  e = 2^1 + 2^11 + 2^12
    int expected = 2 + 2048 + 4096;

    PCUT_ASSERT_INT_EQUALS(expected, RV_J_IMM(instr));
}

PCUT_TEST(j_imm_negative)
{
    rv_instr_t instr;
    instr.j.imm10_1 = -1;
    instr.j.imm11 = -1;
    instr.j.imm19_12 = -1;
    instr.j.imm20 = 1;

    // the result should be sign extended, but with the lowest bit set to zero
    PCUT_ASSERT_INT_EQUALS(-2, (int32_t) RV_J_IMM(instr));
}

PCUT_TEST(b_imm_positive)
{
    rv_instr_t instr;
    instr.b.imm4_1 = 1;
    instr.b.imm10_5 = 1;
    instr.b.imm11 = 1;
    instr.b.imm12 = 0;

    //  e = 2^1 + 2^5 + 2^11
    int expected = 2 + 32 + 2048;

    PCUT_ASSERT_INT_EQUALS(expected, RV_B_IMM(instr));
}

PCUT_TEST(b_imm_negative)
{
    rv_instr_t instr;
    instr.b.imm4_1 = -1;
    instr.b.imm10_5 = -1;
    instr.b.imm11 = -1;
    instr.b.imm12 = 1;

    // Sign extended to -1, then lowest bit set to 0
    PCUT_ASSERT_INT_EQUALS(-2, (int32_t) RV_B_IMM(instr));
}

PCUT_EXPORT(instruction_immediates);
