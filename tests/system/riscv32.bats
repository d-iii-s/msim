#!/usr/bin/env bats

load "common"

@test "RISC-V32: Simple" {
    msim_run_code "riscv32-simple"
}

@test "RISC-V32: Simple with trace" {
    expected=host-trace.expected msim_run_code "riscv32-simple" -t
}
