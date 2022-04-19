#include <stdint.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/computations.h"

PCUT_INIT

PCUT_TEST_SUITE(instruction_decoding);

PCUT_TEST(op_add_decode){
    rv_instr_t instr;
    instr.r.opcode = rv_opcOP;
    instr.r.func3  = 0b111 & rv_func_ADD;
    instr.r.func7  = rv_func_ADD >> 3;

    PCUT_ASSERT_EQUALS(add_instr, rv_instr_decode(instr));
}

PCUT_EXPORT(instruction_decoding);