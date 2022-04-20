#include <stdint.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/computations.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/system.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_decoding);

PCUT_TEST(op_add_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_ADD;
    instr.r.func7  = rv_func_ADD >> 3;

    PCUT_ASSERT_EQUALS(add_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_sub_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SUB;
    instr.r.func7  = rv_func_SUB >> 3;

    PCUT_ASSERT_EQUALS(sub_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_sll_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SLL;
    instr.r.func7  = rv_func_SLL >> 3;

    PCUT_ASSERT_EQUALS(sll_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_slt_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SLT;
    instr.r.func7  = rv_func_SLT >> 3;

    PCUT_ASSERT_EQUALS(slt_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_sltu_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SLTU;
    instr.r.func7  = rv_func_SLTU >> 3;

    PCUT_ASSERT_EQUALS(sltu_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_xor_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_XOR;
    instr.r.func7  = rv_func_XOR >> 3;

    PCUT_ASSERT_EQUALS(xor_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_srl_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SRL;
    instr.r.func7  = rv_func_SRL >> 3;

    PCUT_ASSERT_EQUALS(srl_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_sra_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_SRA;
    instr.r.func7  = rv_func_SRA >> 3;

    PCUT_ASSERT_EQUALS(sra_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_or_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_OR;
    instr.r.func7  = rv_func_OR >> 3;

    PCUT_ASSERT_EQUALS(or_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_and_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_AND;
    instr.r.func7  = rv_func_AND >> 3;

    PCUT_ASSERT_EQUALS(and_instr, rv_instr_decode(instr));
}

PCUT_TEST(op_illegal_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0;
    instr.r.func7  = 0b1010101;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

PCUT_EXPORT(instruction_decoding);