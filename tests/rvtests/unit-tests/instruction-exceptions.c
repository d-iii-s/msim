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

// lwu and swu tests are missing, but I thing this should suffice

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

//? EBREAK

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

PCUT_TEST(sret_trapped) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privSRET
    } };
    cpu.priv_mode = rv_smode;
    // TSR bit => sret traps
    cpu.csr.mstatus |= 1 << 22;

    rv_exc_t ex = sret_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(wfi_trapped_Smode) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privWFI
    } };
    cpu.priv_mode = rv_smode;
    // TW => Trap wait for interrupt (in smode or umode)
    cpu.csr.mstatus |= 1 << 21;

    rv_exc_t ex = wfi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(wfi_trapped_Mmode) {
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .imm = rv_privWFI
    } };
    cpu.priv_mode = rv_mmode;
    // TW => Trap wait for interrupt (in smode or umode)
    cpu.csr.mstatus |= 1 << 21;

    rv_exc_t ex = wfi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(sfence_vma_trapped) {
    rv_instr_t instr = { .r = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcPRIV,
        .funct7 = rv_privSFENCEVMA_FUNCT7
    } };
    cpu.priv_mode = rv_smode;
    // TVM => Trap virtual memory (sfence.vma)
    cpu.csr.mstatus |= 1 << 20;

    rv_exc_t ex = sfence_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
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

PCUT_TEST(csrrw_non_existent_csr){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = 0x6C0, // Custom hypervisor csr in standard,
        .rd = 1
    }};

    rv_exc_t ex = csrrw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_wrong_privilege){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = csr_mie,
        .rs1 = 1,
        .rd  = 2
    }};
    cpu.priv_mode = rv_smode;

    //modifying mmode register in smode
    rv_exc_t ex = csrrw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_write_read_only_csr){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = csr_mhartid,
        .rs1 = 1,
        .rd  = 2
    }};
    cpu.regs[instr.i.rs1] = 5;
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = csrrw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_read_only_csr){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_mhartid,
        .rs1 = 0,
        .rd  = 2
    }};
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrw_WLRL_write_illegal){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = csr_mcause,
        .rs1 = 1,
        .rd  = 2
    }};
    cpu.regs[instr.i.rs1] = RV_EXCEPTION_EXC_BITS | 48; // Exception designated for custom use that is not used int msim
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = csrrw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrw_WARL_write_illegal){
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = csr_mtvec,
        .rs1 = 1,
        .rd  = 2
    }};
    cpu.regs[instr.i.rs1] = 2; // Illegal MODE
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = csrrw_instr(&cpu, instr);

    // No Exception
    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
    // Perserves legal value
    PCUT_ASSERT_INT_EQUALS(cpu.regs[instr.i.rd] & 0b11, cpu.csr.mtvec & 0b11);
}

PCUT_TEST(csrrsi_read_from_disabled_counter_Smode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters disabled
    cpu.csr.mcounteren = 0;
    cpu.priv_mode = rv_smode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_disabled_counter_Mmode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters disabled
    cpu.csr.mcounteren = 0;
    cpu.priv_mode = rv_mmode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrsi_read_from_M_disabled_counter_Umode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters disabled from M mode, but enabled from S mode
    cpu.csr.mcounteren = 0;
    cpu.csr.scounteren = (uint32_t)-1;
    cpu.priv_mode = rv_umode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_S_disabled_counter_Umode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters enabled from M mode, but disabled from S mode
    cpu.csr.mcounteren = (uint32_t)-1;
    cpu.csr.scounteren = 0;
    cpu.priv_mode = rv_umode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_TEST(csrrsi_read_from_enabled_counter_Umode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters enabled
    cpu.csr.mcounteren = (uint32_t)-1;
    cpu.csr.scounteren = (uint32_t)-1;
    cpu.priv_mode = rv_umode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrsi_read_from_M_enabled_S_disabled_counter_Smode){

    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRSI,
        .imm = csr_cycle,
        .rd = 1,
        .rs1 = 0
    }};

    // All counters enabled
    cpu.csr.mcounteren = (uint32_t)-1;
    cpu.csr.scounteren = 0;
    cpu.priv_mode = rv_smode;

    rv_exc_t ex = csrrsi_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_none, ex);
}

PCUT_TEST(csrrw_satp_vm_trapped){
    
    rv_instr_t instr = { .i = {
        .opcode = rv_opcSYSTEM,
        .funct3 = rv_funcCSRRW,
        .imm = csr_satp,
        .rs1 = 1,
        .rd  = 2
    }};
    cpu.priv_mode = rv_smode;
    // TVM => Trap virtual memory (any satp interaction)
    cpu.csr.mstatus |= 1 << 20;

    rv_exc_t ex = csrrw_instr(&cpu, instr);

    PCUT_ASSERT_INT_EQUALS(rv_exc_illegal_instruction, ex);
}

PCUT_EXPORT(instruction_exceptions);