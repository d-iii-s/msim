#include <stdint.h>
#include <pcut/pcut.h>

#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/csr.h"
#include "../../../src/device/cpu/riscv_rv32ima/instructions/system.h"

PCUT_INIT

PCUT_TEST_SUITE(asid_len);

rv_cpu_t cpu0;

PCUT_TEST_BEFORE
{
    rv_cpu_init(&cpu0, 0);
}

PCUT_TEST(default_value)
{
    PCUT_ASSERT_INT_EQUALS(rv_asid_len, cpu0.csr.asid_len);
}

PCUT_TEST(shorter_asid_zeroes_out)
{

    uint32_t ppn = 0x12345;

    // set asid to max value of 511
    cpu0.csr.satp = rv_csr_satp_mode_mask | rv_csr_asid_mask | ppn;

    rv_csr_set_asid_len(&cpu0, 7);

    unsigned expected_asid = 0x7F;

    uint32_t mode_after = cpu0.csr.satp & rv_csr_satp_mode_mask;
    unsigned asid_after = (cpu0.csr.satp & rv_csr_asid_mask) >> rv_csr_satp_asid_offset;
    uint32_t ppn_after = cpu0.csr.satp & rv_csr_satp_ppn_mask;

    PCUT_ASSERT_INT_EQUALS(expected_asid, asid_after);
    PCUT_ASSERT_INT_EQUALS(rv_csr_satp_mode_mask, mode_after);
    PCUT_ASSERT_INT_EQUALS(ppn, ppn_after);
}

PCUT_TEST(longer_asid_perserves)
{

    rv_csr_set_asid_len(&cpu0, 7);

    uint32_t ppn = 0x12345;
    unsigned asid = 0x7F;

    // set asid to max value of 511
    cpu0.csr.satp = rv_csr_satp_mode_mask | (asid << rv_csr_satp_asid_offset) | ppn;

    rv_csr_set_asid_len(&cpu0, 9);

    uint32_t mode_after = cpu0.csr.satp & rv_csr_satp_mode_mask;
    unsigned asid_after = (cpu0.csr.satp & rv_csr_asid_mask) >> rv_csr_satp_asid_offset;
    uint32_t ppn_after = cpu0.csr.satp & rv_csr_satp_ppn_mask;

    PCUT_ASSERT_INT_EQUALS(asid, asid_after);
    PCUT_ASSERT_INT_EQUALS(rv_csr_satp_mode_mask, mode_after);
    PCUT_ASSERT_INT_EQUALS(ppn, ppn_after);
}

PCUT_TEST(csr_write_masks)
{
    rv_csr_set_asid_len(&cpu0, 7);

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_satp,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu0.priv_mode = rv_mmode;

    // Write max asid (0x1FF)
    cpu0.regs[instr.i.rs1] = rv_csr_satp_mode_mask | rv_csr_asid_mask;
    cpu0.regs[instr.i.rd] = 0;

    cpu0.csr.satp = 0;

    rv_exc_t ex = rv_csrrw_instr(&cpu0, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);

    // Only 7 set bits
    unsigned expected_asid = 0x7F;
    unsigned asid_after = (cpu0.csr.satp & rv_csr_asid_mask) >> rv_csr_satp_asid_offset;

    PCUT_ASSERT_INT_EQUALS(expected_asid, asid_after);
}

PCUT_TEST(csr_set_masks)
{
    rv_csr_set_asid_len(&cpu0, 7);

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRS,
                                 .imm = csr_satp,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu0.priv_mode = rv_mmode;

    // Write max asid (0x1FF)
    cpu0.regs[instr.i.rs1] = rv_csr_satp_mode_mask | rv_csr_asid_mask;
    cpu0.regs[instr.i.rd] = 0;

    cpu0.csr.satp = 0;

    rv_exc_t ex = rv_csrrs_instr(&cpu0, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);

    // Only 7 set bits
    unsigned expected_asid = 0x7F;
    unsigned asid_after = (cpu0.csr.satp & rv_csr_asid_mask) >> rv_csr_satp_asid_offset;

    PCUT_ASSERT_INT_EQUALS(expected_asid, asid_after);
}

static unsigned probe_asid_len()
{

    // Write All ones to SATP

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRS,
                                 .imm = csr_satp,
                                 .rs1 = 1,
                                 .rd = 2 } };

    cpu0.regs[instr.i.rs1] = 0xFFFFFFFF;
    cpu0.regs[instr.i.rd] = 0;

    rv_exc_t ex = rv_csrrs_instr(&cpu0, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);

    // Read the content of SATP

    cpu0.regs[instr.i.rs1] = 0;
    cpu0.regs[instr.i.rd] = 0;

    ex = rv_csrrs_instr(&cpu0, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);

    uint32_t satp_content = cpu0.regs[instr.i.rd];

    unsigned asid_mask = (satp_content & rv_csr_asid_mask) >> rv_csr_satp_asid_offset;

    unsigned asid_len = 0;

    while (asid_mask != 0) {
        asid_mask >>= 1;
        asid_len += 1;
    }

    return asid_len;
}

static void test_asid_len_probe(unsigned asid_len)
{
    cpu0.priv_mode = rv_mmode;
    cpu0.csr.satp = 0;

    rv_csr_set_asid_len(&cpu0, asid_len);

    unsigned probed_asid_len = probe_asid_len();

    PCUT_ASSERT_INT_EQUALS(asid_len, probed_asid_len);
}

PCUT_TEST(asid_len_probes)
{
    test_asid_len_probe(9);
    test_asid_len_probe(8);
    test_asid_len_probe(7);
    test_asid_len_probe(6);
    test_asid_len_probe(5);
    test_asid_len_probe(4);
    test_asid_len_probe(3);
    test_asid_len_probe(2);
    test_asid_len_probe(1);
    test_asid_len_probe(0);
}

PCUT_EXPORT(asid_len);