#!/usr/bin/env bats

load "common"

@test "MIPS32: Hello" {
    msim_run_code "mips32-hello"
}

@test "MIPS32: XINT instruction" {
    msim_run_code "mips32-xint"
}

@test "MIPS32: Register dumps" {
    msim_run_code "mips32-rd"
}
