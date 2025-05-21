#include <stdint.h>
#include <pcut/pcut.h>

#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"

#define rv_cpu rv32_cpu
#define rv_cpu_t rv32_cpu_t

// Memory utils
#include "../../../src/device/cpu/riscv_rv_ima/memory.c"

#define rv_convert_addr rv32_convert_addr

// Instructions
#include "../../../src/device/cpu/riscv_rv32ima/instr.c"

PCUT_INIT

PCUT_TEST_SUITE(instruction_exceptions);

rv_cpu_t cpu1;

PCUT_TEST_BEFORE
{
    rv32_cpu_init(&cpu1, 0);
}

PCUT_TEST(add_no_ex)
{
    rv_instr_t instr = { .r = {
                                 .opcode = rv_opcOP,
                                 .funct3 = rv_func_ADD & 0x7,
                                 .funct7 = rv_func_ADD >> 3,
                         } };

    rv_exc_t ex = rv_add_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(jal_address_misaligned)
{
    rv_instr_t instr = { .j = {
                                 .opcode = rv_opcJAL,
                                 .imm10_1 = 1 } };

    rv_exc_t ex = rv_jal_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

PCUT_TEST(jalr_address_misaligned)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcJALR,
                                 .imm = 2 } };

    rv_exc_t ex = rv_jalr_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

// These tests should be ran for all BRANCH instructions, but I think one should suffice

PCUT_TEST(beq_address_misaligned_taken)
{
    rv_instr_t instr = { .b = {
                                 .opcode = rv_opcBRANCH,
                                 .funct3 = rv_func_BEQ,
                                 .rs1 = 0,
                                 .rs2 = 1,
                                 .imm4_1 = 1 } };
    cpu1.regs[0] = 0;
    cpu1.regs[1] = 0;

    rv_exc_t ex = rv_beq_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_instruction_address_misaligned, ex);
}

PCUT_TEST(beq_address_misaligned_not_taken)
{
    rv_instr_t instr = { .b = {
                                 .opcode = rv_opcBRANCH,
                                 .funct3 = rv_func_BEQ,
                                 .rs1 = 0,
                                 .rs2 = 1,
                                 .imm4_1 = 1 } };
    cpu1.regs[0] = 0;
    cpu1.regs[1] = 1;

    rv_exc_t ex = rv_beq_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(lh_address_misaligned)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcLOAD,
                                 .funct3 = rv_func_LH,
                                 .imm = 1 } };

    rv_exc_t ex = rv_lh_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

// lwu and swu tests are missing, but I think this should suffice

PCUT_TEST(lw_address_misaligned)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcLOAD,
                                 .funct3 = rv_func_LW,
                                 .imm = 2 } };

    rv_exc_t ex = rv_lw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

PCUT_TEST(sh_address_misaligned)
{
    rv_instr_t instr = { .s = {
                                 .opcode = rv_opcSTORE,
                                 .funct3 = rv_func_SH,
                                 .imm4_0 = 1 } };

    rv_exc_t ex = rv_sh_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

PCUT_TEST(sw_address_misaligned)
{
    rv_instr_t instr = { .s = {
                                 .opcode = rv_opcSTORE,
                                 .funct3 = rv_func_SW,
                                 .imm4_0 = 2 } };

    rv_exc_t ex = rv_sw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

PCUT_TEST(syscall_umode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privECALL } };
    cpu1.priv_mode = rv_umode;

    rv_exc_t ex = rv_call_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_umode_environment_call, ex);
}

PCUT_TEST(syscall_smode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privECALL } };
    cpu1.priv_mode = rv_smode;

    rv_exc_t ex = rv_call_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_smode_environment_call, ex);
}

//? EBREAK

PCUT_TEST(syscall_mmode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privECALL } };
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_call_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_mmode_environment_call, ex);
}

PCUT_TEST(sret_trapped)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privSRET } };
    cpu1.priv_mode = rv_smode;
    // TSR bit => sret traps
    cpu1.csr.mstatus |= 1 << 22;

    rv_exc_t ex = rv_sret_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(wfi_trapped_Smode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privWFI } };
    cpu1.priv_mode = rv_smode;
    // TW => Trap wait for interrupt (in smode or umode)
    cpu1.csr.mstatus |= 1 << 21;

    rv_exc_t ex = rv_wfi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(wfi_trapped_Mmode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privWFI } };
    cpu1.priv_mode = rv_mmode;
    // TW => Trap wait for interrupt (in smode or umode)
    cpu1.csr.mstatus |= 1 << 21;

    rv_exc_t ex = rv_wfi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(wfi_illegal_Umode)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .imm = rv_privWFI } };
    cpu1.priv_mode = rv_umode;
    // TW => Trap wait for interrupt (clear to 0 - does not trap for S or M mode)
    cpu1.csr.mstatus &= ~(1 << 21);

    rv_exc_t ex = rv_wfi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(sfence_vma_trapped)
{
    rv_instr_t instr = { .r = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcPRIV,
                                 .funct7 = rv_privSFENCEVMA_FUNCT7 } };
    cpu1.priv_mode = rv_smode;
    // TVM => Trap virtual memory (sfence.vma)
    cpu1.csr.mstatus |= 1 << 20;

    rv_exc_t ex = _rv32_sfence_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(lr_address_missaligned)
{
    rv_instr_t instr = { .r = {
                                 .opcode = rv_opcAMO,
                                 .funct7 = rv_funcLR << 2,
                                 .funct3 = RV_AMO_32_WLEN,
                                 .rs1 = 0,
                         } };

    cpu1.regs[0] = 2;

    rv_exc_t ex = rv_lr_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_load_address_misaligned, ex);
}

PCUT_TEST(sc_address_missaligned)
{
    rv_instr_t instr = { .r = {
                                 .opcode = rv_opcAMO,
                                 .funct7 = rv_funcSC << 2,
                                 .funct3 = RV_AMO_32_WLEN,
                                 .rs1 = 0,
                         } };

    cpu1.regs[0] = 2;
    cpu1.reserved_valid = true;

    rv_exc_t ex = rv_sc_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

// this should be done for all amo instructions, but I think one should suffice
PCUT_TEST(amo_address_missaligned)
{
    rv_instr_t instr = { .r = {
                                 .opcode = rv_opcAMO,
                                 .funct7 = rv_funcAMOSWAP << 2,
                                 .funct3 = RV_AMO_32_WLEN,
                                 .rs1 = 0,
                         } };

    cpu1.regs[0] = 2;

    rv_exc_t ex = rv_amoswap_w_instr(&cpu1, instr);
    PCUT_ASSERT_INT_EQUALS(rv_exc_store_amo_address_misaligned, ex);
}

PCUT_TEST(csrrw_non_existent_csr)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = 0x6C0, // Custom hypervisor csr in standard,
                                 .rd = 1 } };

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_wrong_privilege)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_mie,
                                 .rs1 = 0,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_smode;

    // modifying mmode register in smode
    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_write_read_only_csr)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_mhartid,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.regs[instr.i.rs1] = 5;
    cpu1.regs[instr.i.rd] = (uint32_t) -1;
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
    // value in rd didn't change
    PCUT_ASSERT_INT_EQUALS((uint32_t) -1, cpu1.regs[instr.i.rd]);
}

PCUT_TEST(csrrsi_read_read_only_csr)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_mhartid,
                                 .rs1 = 0,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrw_WLRL_write_illegal)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_mcause,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.regs[instr.i.rs1] = RV_EXCEPTION_EXC_BITS | 48; // Exception designated for custom use that is not used int msim
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_WARL_write_illegal)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_mtvec,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.regs[instr.i.rs1] = 2; // Illegal MODE
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    // No Exception
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
    // Perserves legal value
    PCUT_ASSERT_INT_EQUALS(cpu1.regs[instr.i.rd] & 0b11, cpu1.csr.mtvec & 0b11);
}

PCUT_TEST(csrrsi_read_from_disabled_counter_Smode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters disabled
    cpu1.csr.mcounteren = 0;
    cpu1.priv_mode = rv_smode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_disabled_counter_Mmode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters disabled
    cpu1.csr.mcounteren = 0;
    cpu1.priv_mode = rv_mmode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrsi_read_from_M_disabled_counter_Umode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters disabled from M mode, but enabled from S mode
    cpu1.csr.mcounteren = 0;
    cpu1.csr.scounteren = (uint32_t) -1;
    cpu1.priv_mode = rv_umode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_S_disabled_counter_Umode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters enabled from M mode, but disabled from S mode
    cpu1.csr.mcounteren = (uint32_t) -1;
    cpu1.csr.scounteren = 0;
    cpu1.priv_mode = rv_umode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_enabled_counter_Umode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters enabled
    cpu1.csr.mcounteren = (uint32_t) -1;
    cpu1.csr.scounteren = (uint32_t) -1;
    cpu1.priv_mode = rv_umode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrsi_read_from_M_enabled_S_disabled_counter_Smode)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRSI,
                                 .imm = csr_cycle,
                                 .rd = 1,
                                 .rs1 = 0 } };

    // All counters enabled
    cpu1.csr.mcounteren = (uint32_t) -1;
    cpu1.csr.scounteren = 0;
    cpu1.priv_mode = rv_smode;

    rv_exc_t ex = rv_csrrsi_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrw_satp_vm_trapped)
{

    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_satp,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_smode;
    // TVM => Trap virtual memory (any satp interaction)
    cpu1.csr.mstatus |= 1 << 20;

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_WPRI_field_ignores_writes)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRW,
                                 .imm = csr_mstatus,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_mmode;

    cpu1.regs[instr.i.rs1] = 0xFFFFFFFF;
    cpu1.regs[instr.i.rd] = (uint32_t) -1;

    // We expect these fields to be set to 1 and the rest to stay at 0
    uint64_t expected_mstatus = rv_csr_mstatus_mask;

    rv_exc_t ex = rv_csrrw_instr(&cpu1, instr);

    // No exception should occur
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
    PCUT_ASSERT_INT_EQUALS(expected_mstatus, cpu1.csr.mstatus);
}

PCUT_TEST(csrrs_WPRI_field_ignores_writes)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRS,
                                 .imm = csr_mstatus,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_mmode;

    cpu1.regs[instr.i.rs1] = 0xFFFFFFFF;
    cpu1.regs[instr.i.rd] = (uint32_t) -1;

    // We expect these fields to be set to 1 and the rest to stay at 0
    uint64_t expected_mstatus = rv_csr_mstatus_mask;

    rv_exc_t ex = rv_csrrs_instr(&cpu1, instr);

    // No exception should occur
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
    PCUT_ASSERT_INT_EQUALS(expected_mstatus, cpu1.csr.mstatus);
}

PCUT_TEST(csrrc_WPRI_field_ignores_writes)
{
    rv_instr_t instr = { .i = {
                                 .opcode = rv_opcSYSTEM,
                                 .funct3 = rv_funcCSRRC,
                                 .imm = csr_mstatus,
                                 .rs1 = 1,
                                 .rd = 2 } };
    cpu1.priv_mode = rv_mmode;

    cpu1.regs[instr.i.rs1] = 0xFFFFFFFF;
    cpu1.regs[instr.i.rd] = (uint32_t) -1;

    // We expect the legal fields to be reset to 0, while the rest stays at 1
    uint64_t expected_mstatus = (uint32_t) (~rv_csr_mstatus_mask);

    // This is an illegal value only used for this test
    cpu1.csr.mstatus = 0xFFFFFFFF;
    rv_exc_t ex = rv_csrrc_instr(&cpu1, instr);

    // No exception should occur
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
    PCUT_ASSERT_INT_EQUALS(expected_mstatus, cpu1.csr.mstatus);
}

PCUT_EXPORT(instruction_exceptions);

#undef rv_cpu
#undef rv_cpu_t
#undef rv_convert_addr
