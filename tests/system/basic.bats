#!/usr/bin/env bats

load "common"

@test "Version switch" {
    run "$MSIM" --version
    test "$status" -eq 0
    if ! echo "$output" | egrep -q '^MSIM version [1-9][0-9]*\.[0-9]+\.[0-9]+$'; then
        fail "Failure: version string not found in output: '$output'."
    fi
}

@test "Empty run" {
    config="" expected="" msim_command_check
}

@test "Cannot add device with command name" {
    config="
        add dr4kcpu add
        dumpdev
    " \
    expected="
        <msim> Error in msim.conf on line 1:
        Device name \"add\" is in conflict with a command name
        <msim> Fault in msim.conf on line 1:
        Error in configuration file
    " \
    exit_success=false \
    msim_command_check
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

@test "Add SH-2E CPU" {
    config="
        add dsh2ecpu sh2e
        sh2e info
        dumpdev
    " \
    expected="
        SH-2E
        [  name  ] [  type  ] [ parameters...
        sh2e       dsh2ecpu   SH-2E
    " \
    msim_command_check
}

@test "Empty machine device dump" {
    config="
        dumpdev
    " \
    expected="
        [  name  ] [  type  ] [ parameters...
        No matching devices found.
    " \
    msim_command_check
}

@test "Cannot add device with same name twice" {
    config="
        add dr4kcpu mips
        add dr4kcpu mips
        dumpdev
    " \
    expected="
        <msim> Error in msim.conf on line 2:
        Device name \"mips\" already added
        <msim> Fault in msim.conf on line 2:
        Error in configuration file
    " \
    exit_success=false \
    msim_command_check
}

@test "Invalid call to add terminates with error (issue 44)" {
    config="
        add ddisk disk
    " \
    expected="
        <msim> Error in msim.conf on line 1:
        Missing parameter \"register block address\"
        <msim> Fault in msim.conf on line 1:
        Error in configuration file
    " \
    exit_success=false \
    msim_command_check
}
