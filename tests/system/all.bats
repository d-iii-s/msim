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

@test "MIPS 32: issue/6-mips-bad-status-ksu" {
    msim_run_code "images/issue__6_mips_bad_status_ksu__mips32"
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

@test "RISC-V 32: riscv32/amo" {
    msim_run_code "images/riscv32__amo__riscv32"
}

@test "RISC-V 32: riscv32/basic" {
    msim_run_code "images/riscv32__basic__riscv32"
}

@test "RISC-V 32: riscv32/branches" {
    msim_run_code "images/riscv32__branches__riscv32"
}

@test "RISC-V 32: riscv32/exc-delegated" {
    msim_run_code "images/riscv32__exc_delegated__riscv32"
}

@test "RISC-V 32: riscv32/exc-delegated-from-m" {
    msim_run_code "images/riscv32__exc_delegated_from_m__riscv32"
}

@test "RISC-V 32: riscv32/exc-interrupt-delegated" {
    msim_run_code "images/riscv32__exc_interrupt_delegated__riscv32"
}

@test "RISC-V 32: riscv32/exc-interrupt-delegation-masks" {
    msim_run_code "images/riscv32__exc_interrupt_delegation_masks__riscv32"
}

@test "RISC-V 32: riscv32/exc-interrupt-simple" {
    msim_run_code "images/riscv32__exc_interrupt_simple__riscv32"
}

@test "RISC-V 32: riscv32/exc-not-delegated" {
    msim_run_code "images/riscv32__exc_not_delegated__riscv32"
}

@test "RISC-V 32: riscv32/exc-simple" {
    msim_run_code "images/riscv32__exc_simple__riscv32"
}

@test "RISC-V 32: riscv32/jumps" {
    msim_run_code "images/riscv32__jumps__riscv32"
}

@test "RISC-V 32: riscv32/loads" {
    msim_run_code "images/riscv32__loads__riscv32"
}

@test "RISC-V 32: riscv32/lr-sc" {
    msim_run_code "images/riscv32__lr_sc__riscv32"
}

@test "RISC-V 32: riscv32/m-extension" {
    msim_run_code "images/riscv32__m_extension__riscv32"
}

@test "RISC-V 32: riscv32/m-mode-stip" {
    msim_run_code "images/riscv32__m_mode_stip__riscv32"
}

@test "RISC-V 32: riscv32/mprv-fetch" {
    msim_run_code "images/riscv32__mprv_fetch__riscv32"
}

@test "RISC-V 32: riscv32/op-imm" {
    msim_run_code "images/riscv32__op_imm__riscv32"
}

@test "RISC-V 32: riscv32/ops" {
    msim_run_code "images/riscv32__ops__riscv32"
}

@test "RISC-V 32: riscv32/scyclecmp" {
    msim_run_code "images/riscv32__scyclecmp__riscv32"
}

@test "RISC-V 32: riscv32/simple" {
    msim_run_code "images/riscv32__simple__riscv32"
}

@test "RISC-V 32: riscv32/stores" {
    msim_run_code "images/riscv32__stores__riscv32"
}

@test "RISC-V 32: riscv32/tlb" {
    msim_run_code "images/riscv32__tlb__riscv32"
}

@test "RISC-V 32: riscv32/virtual-addressing" {
    msim_run_code "images/riscv32__virtual_addressing__riscv32"
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

@test "SYSTEM: system/external-seip" {
     msim_run_sys "images/system__external_seip__sys"
}

