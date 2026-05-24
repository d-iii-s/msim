#!/usr/bin/env bats

load "common"

@test "DAP RISC-V32: Terminate" {
    msim_dap_run "dap-simple-riscv32" "scenario_terminate.py"
}

@test "DAP RISC-V32: Step" {
    msim_dap_run "dap-simple-riscv32" "scenario_step.py"
}

@test "DAP RISC-V32: Resume" {
    msim_dap_run "dap-simple-riscv32" "scenario_resume.py"
}

@test "DAP RISC-V32: Code Breakpoint" {
    msim_dap_run "dap-simple-riscv32" "scenario_breakpoint.py"
}

@test "DAP RISC-V32: Data Breakpoint" {
    msim_dap_run "dap-simple-riscv32" "scenario_data_breakpoint.py"
}

@test "DAP RISC-V32: Register" {
    msim_dap_run "dap-simple-riscv32" "scenario_register.py"
}

@test "DAP RISC-V32: CSR" {
    msim_dap_run "dap-simple-riscv32" "scenario_csr.py"
}

@test "DAP RISC-V32: PC" {
    msim_dap_run "dap-simple-riscv32" "scenario_pc.py"
}

@test "DAP RISC-V32: CPU info" {
    msim_dap_run "dap-simple-riscv32" "scenario_cpu_info.py"
}

@test "DAP RISC-V32: Memory" {
    msim_dap_run "dap-simple-riscv32" "scenario_mem.py"
}

@test "DAP RISC-V32: Interrupts" {
    msim_dap_run "dap-simple-riscv32" "scenario_interrupts.py"
}

@test "DAP RISC-V32: Bad request" {
    msim_dap_run "dap-simple-riscv32" "scenario_bad_req.py"
}

@test "DAP RISC-V64: Terminate" {
    msim_dap_run "dap-simple-riscv64" "scenario_terminate.py"
}

@test "DAP RISC-V64: Step" {
    msim_dap_run "dap-simple-riscv64" "scenario_step.py"
}

@test "DAP RISC-V64: Resume" {
    msim_dap_run "dap-simple-riscv64" "scenario_resume.py"
}

@test "DAP RISC-V64: Code Breakpoint" {
    msim_dap_run "dap-simple-riscv64" "scenario_breakpoint.py"
}

@test "DAP RISC-V64: Data Breakpoint" {
    msim_dap_run "dap-simple-riscv64" "scenario_data_breakpoint.py"
}

@test "DAP RISC-V64: Register" {
    msim_dap_run "dap-simple-riscv64" "scenario_register.py"
}

@test "DAP RISC-V64: CSR" {
    msim_dap_run "dap-simple-riscv64" "scenario_csr.py"
}

@test "DAP RISC-V64: PC" {
    msim_dap_run "dap-simple-riscv64" "scenario_pc.py"
}

@test "DAP RISC-V64: CPU info" {
    msim_dap_run "dap-simple-riscv64" "scenario_cpu_info.py"
}

@test "DAP RISC-V64: Memory" {
    msim_dap_run "dap-simple-riscv64" "scenario_mem.py"
}

@test "DAP RISC-V64: Interrupts" {
    msim_dap_run "dap-simple-riscv64" "scenario_interrupts.py"
}

@test "DAP RISC-V64: Bad request" {
    msim_dap_run "dap-simple-riscv64" "scenario_bad_req.py"
}

@test "DAP MIPS32: Terminate" {
    msim_dap_run "dap-simple-mips32" "scenario_terminate.py"
}

@test "DAP MIPS32: Step" {
    msim_dap_run "dap-simple-mips32" "scenario_step.py"
}

@test "DAP MIPS32: Resume" {
    msim_dap_run "dap-simple-mips32" "scenario_resume.py"
}

@test "DAP MIPS32: Code Breakpoint" {
    msim_dap_run "dap-simple-mips32" "scenario_breakpoint.py"
}

@test "DAP MIPS32: Data Breakpoint" {
    msim_dap_run "dap-simple-mips32" "scenario_data_breakpoint.py"
}

@test "DAP MIPS32: Register" {
    msim_dap_run "dap-simple-mips32" "scenario_register.py"
}

@test "DAP MIPS32: CSR" {
    msim_dap_run "dap-simple-mips32" "scenario_csr.py"
}

@test "DAP MIPS32: PC" {
    msim_dap_run "dap-simple-mips32" "scenario_pc.py"
}

@test "DAP MIPS32: CPU info" {
    msim_dap_run "dap-simple-mips32" "scenario_cpu_info.py"
}

@test "DAP MIPS32: Memory" {
    msim_dap_run "dap-simple-mips32" "scenario_mem.py"
}

@test "DAP MIPS32: Interrupts" {
    msim_dap_run "dap-simple-mips32" "scenario_interrupts.py"
}

@test "DAP MIPS32: Bad request" {
    msim_dap_run "dap-simple-mips32" "scenario_bad_req.py"
}

@test "DAP Multiarch (RV32+RV64): Terminate" {
    msim_dap_run "dap-simple-multiarch" "scenario_terminate.py"
}

@test "DAP Multiarch (RV32+RV64): Step" {
    msim_dap_run "dap-simple-multiarch" "scenario_step.py"
}

@test "DAP Multiarch (RV32+RV64): Resume" {
    msim_dap_run "dap-simple-multiarch" "scenario_resume.py"
}

@test "DAP Multiarch (RV32+RV64): Code Breakpoint" {
    msim_dap_run "dap-simple-multiarch" "scenario_breakpoint.py"
}

@test "DAP Multiarch (RV32+RV64): Data Breakpoint" {
    msim_dap_run "dap-simple-multiarch" "scenario_data_breakpoint.py"
}

@test "DAP Multiarch (RV32+RV64): Register" {
    msim_dap_run "dap-simple-multiarch" "scenario_register.py"
}

@test "DAP Multiarch (RV32+RV64): CSR" {
    msim_dap_run "dap-simple-multiarch" "scenario_csr.py"
}

@test "DAP Multiarch (RV32+RV64): PC" {
    msim_dap_run "dap-simple-multiarch" "scenario_pc.py"
}

@test "DAP Multiarch (RV32+RV64): Interrupts" {
    msim_dap_run "dap-simple-multiarch" "scenario_interrupts.py"
}

@test "DAP Multiarch (RV32+RV64): CPU info" {
    msim_dap_run "dap-simple-multiarch" "scenario_cpu_info.py"
}
