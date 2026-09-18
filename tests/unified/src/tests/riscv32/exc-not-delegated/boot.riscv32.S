.text
#include "marker_handler.riscv32.inc"

la t0, failure_handler
csrw stvec, t0
la t0, success_handler
csrw mtvec, t0
la t1, resume
csrw mepc, t1
// Returns to U-mode
mret
resume:
csrrw x1, sepc, t1
li t1, 'F'
sw t1, (t0)
simulator_halt

success_handler:
    write_marker_and_halt 'S'

failure_handler:
    write_marker_and_halt 'F'
