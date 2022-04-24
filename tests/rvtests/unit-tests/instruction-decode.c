#include <stdint.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/computations.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/control_transfer.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/mem_ops.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/system.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_decoding);

/*******************
 * OP instructions *
 *******************/

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

//TODO: M extension instructions

PCUT_TEST(op_illegal_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0;
    instr.r.func7  = 0b1010101;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

/***********************
 * BRANCH instructions *
 ***********************/

PCUT_TEST(branch_beq_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BEQ;

    PCUT_ASSERT_EQUALS(beq_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_bne_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BNE;

    PCUT_ASSERT_EQUALS(bne_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_blt_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BLT;

    PCUT_ASSERT_EQUALS(blt_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_bltu_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BLTU;

    PCUT_ASSERT_EQUALS(bltu_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_bge_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BGE;

    PCUT_ASSERT_EQUALS(bge_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_bgeu_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = rv_func_BGEU;

    PCUT_ASSERT_EQUALS(bgeu_instr, rv_instr_decode(instr));
}

PCUT_TEST(branch_illegal_decode){
    rv_instr_t instr;
    instr.b.opcode = rv_opcBRANCH;
    instr.b.func3 = 0b010;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

/********************
 * JALR instruction *
 ********************/

PCUT_TEST(jalr_decode){
    rv_instr_t instr;
    instr.j.opcode = rv_opcJALR;
    instr.b.func3 = 0;

    PCUT_ASSERT_EQUALS(jalr_instr, rv_instr_decode(instr));
}

PCUT_TEST(jalr_illegal_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcJALR;
    instr.i.func3 = 3;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

/*******************
 * JAL instruction *
 *******************/

PCUT_TEST(jal_decode){
    rv_instr_t instr;
    instr.j.opcode = rv_opcJAL;

    PCUT_ASSERT_EQUALS(jal_instr, rv_instr_decode(instr));
}

//TODO: Rest of tests

/*********************
 * LOAD instructions *
 *********************/

PCUT_TEST(lb_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = rv_func_LB;

    PCUT_ASSERT_EQUALS(lb_instr, rv_instr_decode(instr));
}

PCUT_TEST(lh_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = rv_func_LH;

    PCUT_ASSERT_EQUALS(lh_instr, rv_instr_decode(instr));
}

PCUT_TEST(lw_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = rv_func_LW;

    PCUT_ASSERT_EQUALS(lw_instr, rv_instr_decode(instr));
}

PCUT_TEST(lbu_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = rv_func_LBU;

    PCUT_ASSERT_EQUALS(lbu_instr, rv_instr_decode(instr));
}

PCUT_TEST(lhu_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = rv_func_LHU;

    PCUT_ASSERT_EQUALS(lhu_instr, rv_instr_decode(instr));
}

PCUT_TEST(load_illegal_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcLOAD;
    instr.i.func3  = 0b111;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

/**********************
 * STORE instructions *
 **********************/

/*************************
 * MISC MEM instructions *
 *************************/

PCUT_TEST(fence_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcMISC_MEM;
    instr.i.func3 = 0;

    PCUT_ASSERT_EQUALS(fence_instr, rv_instr_decode(instr));
}

PCUT_TEST(misc_mem_illegal_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcMISC_MEM;
    instr.i.func3 = 2;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

/***********************
 * OP IMM instructions *
 ***********************/

PCUT_TEST(addi_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_ADDI;

    PCUT_ASSERT_EQUALS(addi_instr, rv_instr_decode(instr));
}

PCUT_TEST(slti_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SLTI;

    PCUT_ASSERT_EQUALS(slti_instr, rv_instr_decode(instr));
}

PCUT_TEST(sltiu_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SLTIU;

    PCUT_ASSERT_EQUALS(sltiu_instr, rv_instr_decode(instr));
}

PCUT_TEST(xori_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_XORI;

    PCUT_ASSERT_EQUALS(xori_instr, rv_instr_decode(instr));
}

PCUT_TEST(ori_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_ORI;

    PCUT_ASSERT_EQUALS(ori_instr, rv_instr_decode(instr));
}

PCUT_TEST(andi_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_ANDI;

    PCUT_ASSERT_EQUALS(andi_instr, rv_instr_decode(instr));
}

PCUT_TEST(slli_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SLLI;
    instr.i.imm = 0;

    PCUT_ASSERT_EQUALS(slli_instr, rv_instr_decode(instr));
}

PCUT_TEST(slli_illegal_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SLLI;
    instr.i.imm = 1<<6;

    PCUT_ASSERT_EQUALS(illegal_instr, rv_instr_decode(instr));
}

PCUT_TEST(srli_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SRI;
    instr.i.imm = 0;

    PCUT_ASSERT_EQUALS(srli_instr, rv_instr_decode(instr));
}

PCUT_TEST(srai_decode){
    rv_instr_t instr;
    instr.i.opcode = rv_opcOP_IMM;
    instr.i.func3 = rv_func_SRI;
    // set bit 30, as in spec
    instr.val |= 1<<30;

    PCUT_ASSERT_EQUALS(srai_instr, rv_instr_decode(instr));
}

/**********************
 * AUIPC instructions *
 **********************/

PCUT_TEST(auipc_decode){
    rv_instr_t instr;
    instr.u.opcode = rv_opcAUIPC;

    PCUT_ASSERT_EQUALS(auipc_instr, rv_instr_decode(instr));
}

/********************
 * AMO instructions *
 ********************/

/********************
 * LUI instructions *
 ********************/

PCUT_TEST(lui_decode){
    rv_instr_t instr;
    instr.u.opcode = rv_opcLUI;

    PCUT_ASSERT_EQUALS(lui_instr, rv_instr_decode(instr));
}

/********************
 * OP_32 instructions *
 ********************/

/***********************
 * SYSTEM instructions *
 ***********************/

PCUT_EXPORT(instruction_decoding);