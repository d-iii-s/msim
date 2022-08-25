#include <stdint.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/csr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/computations.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/control_transfer.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/mem_ops.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/system.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_exceptions);

rv_cpu_t cpu;

PCUT_TEST_BEFORE {
    rv_cpu_init(&cpu, 0);
}

PCUT_TEST(add_no_ex){
    rv_instr_t instr = { .r = {
        .opcode = rv_opcOP,
        .funct3 = rv_func_ADD & 0x7,
        .funct7 = rv_func_ADD >> 3,
    } };

    rv_exc_t ex = add_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(jal_address_misaligned){
    rv_instr_t instr = { .j = {
        .opcode = rv_opcJAL,
        .imm10_1 = 1
    } };

    rv_exc_t ex = jal_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

PCUT_TEST(jalr_address_misaligned){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcJALR,
        .imm = 2
    } };

    rv_exc_t ex = jalr_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

// These tests should be ran for all BRANCH instructions, but I think one should suffice

PCUT_TEST(beq_address_misaligned_taken){
    rv_instr_t instr = { .b = {
        .opcode = rv_opcBRANCH,
        .funct3 = rv_func_BEQ,
        .rs1 = 0,
        .rs2 = 1,
        .imm4_1 = 1
    } };
    cpu.regs[0] = 0;
    cpu.regs[1] = 0;

    rv_exc_t ex = beq_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

PCUT_TEST(beq_address_misaligned_not_taken){
    rv_instr_t instr = { .b = {
        .opcode = rv_opcBRANCH,
        .funct3 = rv_func_BEQ,
        .rs1 = 0,
        .rs2 = 1,
        .imm4_1 = 1
    } };
    cpu.regs[0] = 0;
    cpu.regs[1] = 1;

    rv_exc_t ex = beq_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(lh_address_misaligned){
     rv_instr_t instr = { .i = {
        .opcode = rv_opcLOAD,
        .funct3 = rv_func_LH,
        .imm = 1
    } };

    rv_exc_t ex = lh_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

PCUT_TEST(lw_address_misaligned){
     rv_instr_t instr = { .i = {
        .opcode = rv_opcLOAD,
        .funct3 = rv_func_LW,
        .imm = 2
    } };

    rv_exc_t ex = lw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

PCUT_TEST(sh_address_misaligned){
     rv_instr_t instr = { .s = {
        .opcode = rv_opcSTORE,
        .funct3 = rv_func_SH,
        .imm4_0 = 1
    } };

    rv_exc_t ex = sh_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

PCUT_TEST(sw_address_misaligned){
     rv_instr_t instr = { .s = {
        .opcode = rv_opcSTORE,
        .funct3 = rv_func_SW,
        .imm4_0 = 2
    } };

    rv_exc_t ex = sw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

PCUT_TEST(syscall_umode) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privECALL
    } };
    cpu.priv_mode = rv_umode;

    rv_exc_t ex = call_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_umode_environment_call, ex);
}

PCUT_TEST(syscall_smode) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privECALL
    } };
    cpu.priv_mode = rv_smode;

    rv_exc_t ex = call_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_smode_environment_call, ex);
}

PCUT_TEST(syscall_mmode) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privECALL
    } };
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = call_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_mmode_environment_call, ex);
}

PCUT_TEST(lr_address_missaligned) {
    rv_instr_t instr = { .r = {
        .opcode = rv_opcAMO,
        .funct7 = rv_funcLR << 2,
        .funct3 = RV_AMO_32_WLEN,
        .rs1 = 0,
    }};

    cpu.regs[0] = 2;

    rv_exc_t ex = lr_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

PCUT_TEST(sc_address_missaligned) {
    rv_instr_t instr = { .r = {
        .opcode = rv_opcAMO,
        .funct7 = rv_funcSC << 2,
        .funct3 = RV_AMO_32_WLEN,
        .rs1 = 0,
    }};

    cpu.regs[0] = 2;

    rv_exc_t ex = sc_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

// this should be done for all amo instructions, but I think one should suffice
PCUT_TEST(amo_address_missaligned) {
    rv_instr_t instr = { .r = {
        .opcode = rv_opcAMO,
        .funct7 = rv_funcAMOSWAP << 2,
        .funct3 = RV_AMO_32_WLEN,
        .rs1 = 0,
    }};

    cpu.regs[0] = 2;

    rv_exc_t ex = amoswap_instr(&cpu, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

// TODO: CSR

PCUT_EXPORT(instruction_exceptions);