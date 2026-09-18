.text
#include "marker_handler.riscv32.inc"

la t0, failure_handler
csrw stvec, t0
la t0, success_handler
csrw mtvec, t0
csrsi medeleg, 4
// Should trap to M mode
csrrw x1, mvendorid, t1
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
simulator_halt

success_handler:
    write_marker_and_halt 'S'

failure_handler:
    write_marker_and_halt 'F'
