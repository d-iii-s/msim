#define mstatus_mpp_s (0b01<<11)
#define mstatus_mprv (1<<17)
.text
#include "marker_handler.riscv32.inc"

// tests that instruction fetches are not translated even when mstatus.MPRV is set

// Setup a failing exception handler
la t0, failure_handler
csrw mtvec, t0

// load satp with some bogus value that should result in an page fault if the instruction would be fetched from it
li t0, 0x80000000
csrw satp, t0

// set mstatus to use S-level memory translation
li t0, mstatus_mpp_s | mstatus_mprv
csrs mstatus, t0

// Perform an instruction, should not pagefault
addi t0, t1, 0

// set mstatus back to no translation
li t0, mstatus_mprv
csrc mstatus, t0

// Print success and halt
li t0, 0x90000000
li t1, 'S'
sw t1, (t0)
simulator_halt

failure_handler:
    write_marker_and_halt 'F'
