#!/usr/bin/env bats

# Generated file. Do not edit but commit

load "../system/common"


@test "MIPS 32: basic/interactive" {
    msim_run_code "images/basic__interactive__mips32"
}

@test "RISC-V 32: basic/interactive" {
    msim_run_code "images/basic__interactive__riscv32"
}

@test "MIPS 32: basic/printer" {
    msim_run_code "images/basic__printer__mips32"
}

@test "RISC-V 32: basic/printer" {
    msim_run_code "images/basic__printer__riscv32"
}

@test "MIPS 32: basic/tracing" {
    msim_run_code "images/basic__tracing__mips32"
}

@test "RISC-V 32: basic/tracing" {
    msim_run_code "images/basic__tracing__riscv32"
}

@test "MIPS 32: dnomem/break" {
    msim_run_code "images/dnomem__break__mips32"
}

@test "RISC-V 32: dnomem/break" {
    msim_run_code "images/dnomem__break__riscv32"
}

@test "MIPS 32: dnomem/halt" {
    msim_run_code "images/dnomem__halt__mips32"
}

@test "RISC-V 32: dnomem/halt" {
    msim_run_code "images/dnomem__halt__riscv32"
}

@test "MIPS 32: dnomem/regdump" {
    msim_run_code "images/dnomem__regdump__mips32"
}

@test "RISC-V 32: dnomem/regdump" {
    msim_run_code "images/dnomem__regdump__riscv32"
}

@test "MIPS 32: dnomem/warn" {
    msim_run_code "images/dnomem__warn__mips32"
}

@test "RISC-V 32: dnomem/warn" {
    msim_run_code "images/dnomem__warn__riscv32"
}

@test "MIPS 32: mips32/dval" {
    msim_run_code "images/mips32__dval__mips32"
}

@test "MIPS 32: regdump/after-boot" {
    msim_run_code "images/regdump__after_boot__mips32"
}

@test "RISC-V 32: regdump/after-boot" {
    msim_run_code "images/regdump__after_boot__riscv32"
}

@test "RISC-V 32: riscv32/basic" {
    msim_run_code "images/riscv32__basic__riscv32"
}

