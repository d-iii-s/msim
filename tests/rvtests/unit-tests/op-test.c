#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/instr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/computations.h"

PCUT_INIT

PCUT_TEST_SUITE(Ops);

PCUT_TEST(add_basic) {
    //TODO: implement real test
    PCUT_ASSERT_INT_EQUALS(42, 42);
}

PCUT_EXPORT(Ops);