#!/usr/bin/env bats

# Generated file. Do not edit but commit

load "../system/common"


@test "MIPS 32: regdump/after-boot" {
    msim_run_code "images/regdump__after_boot__mips32"
}

@test "RISC-V 32: regdump/after-boot" {
    msim_run_code "images/regdump__after_boot__riscv32"
}

@test "RISC-V 32: riscv32/basic" {
    msim_run_code "images/riscv32__basic__riscv32"
}

