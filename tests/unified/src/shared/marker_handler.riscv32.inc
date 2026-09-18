.macro write_marker_and_halt char
    li t0, 0x90000000
    li t1, \char
    sw t1, 0(t0)
    simulator_halt
.endm write_marker_and_halt
