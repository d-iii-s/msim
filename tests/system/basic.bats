#!/usr/bin/env bats

load "common.bash"

@test "Version switch" {
    run "$MSIM" --version
    test "$status" -eq 0
    if ! echo "$output" | egrep -q '^MSIM version [1-9][0-9]*\.[0-9]+\.[0-9]+$'; then
        fail "Failure: version string not found in output: '$output'."
    fi
}

@test "Add R4000 MIPS CPU" {
    config="
        add dr4kcpu mips
        mips info
        dumpdev
    " \
    expected="
        R4000
        [  name  ] [  type  ] [ parameters...
        mips       dr4kcpu    R4000
    " \
    msim_command_check
}

@test "Add RISC-V CPU" {
    config="
        add drvcpu riscv
        riscv info
        dumpdev
    " \
    expected="
        RV32IMA (processor ID: 0)
        [  name  ] [  type  ] [ parameters...
        riscv      drvcpu     RV32IMA (processor ID: 0)
    " \
    msim_command_check
}
