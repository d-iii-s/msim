#!/usr/bin/env bats

load "common"

@test "MIPS32: Hello" {
    msim_run_code "mips32-hello"
}

@test "MIPS32: dnomem device in warn mode" {
    msim_run_code "mips32-dnomem-warn"
}

@test "MIPS32: dnomem device in break mode" {
    msim_run_code "mips32-dnomem-break"
}

@test "MIPS32: dnomem device in halt mode" {
    msim_run_code "mips32-dnomem-halt"
}

@test "MIPS32: dnomem device in halt mode with register dump" {
    msim_run_code "mips32-dnomem-rd"
}

@test "MIPS32: XINT instruction" {
    msim_run_code "mips32-xint"
}

@test "MIPS32: Register dumps" {
    msim_run_code "mips32-rd"
}
