.text
#include "marker_handler.riscv32.inc"

la t0, success_handler
csrw stvec, t0
la t0, failure_handler
csrw mtvec, t0
// Enable SSI
csrwi mie, 2
// Set SSI pending
csrsi mip, 2
// Delegate SSI
csrsi mideleg, 2
// Enable supervisor interrupts
csrsi mstatus, 2
// Set MPP as S mode
li t0, 1 << 11
csrs mstatus, t0
// Setup mret address
la t0, resume
csrw mepc, t0
// mret to S mode, should raise interrupt
mret
resume:
// Fail
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
simulator_halt

success_handler:
    write_marker_and_halt 'S'

failure_handler:
    write_marker_and_halt 'F'
