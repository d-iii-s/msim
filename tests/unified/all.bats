#!/usr/bin/env bats

# Generated file. Do not edit but commit

load "common"


@test "SYSTEM: basic/duplicate-device-name" {
     exit_success=false msim_run_sys "images/basic__duplicate_device_name__sys"
}

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

@test "SYSTEM: issue/44-invalid-add-call" {
     exit_success=false msim_run_sys "images/issue__44_invalid_add_call__sys"
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

@test "SYSTEM: system/bad-device-name" {
     exit_success=false msim_run_sys "images/system__bad_device_name__sys"
}

@test "SYSTEM: system/dumpdev-empty" {
     msim_run_sys "images/system__dumpdev_empty__sys"
}

@test "SYSTEM: system/dumpdev-mips" {
     msim_run_sys "images/system__dumpdev_mips__sys"
}

@test "SYSTEM: system/dumpdev-rv32" {
     msim_run_sys "images/system__dumpdev_rv32__sys"
}

@test "SYSTEM: system/empty" {
     msim_run_sys "images/system__empty__sys"
}

