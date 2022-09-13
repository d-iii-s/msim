.text
li t0, 0x80000000
csrw stvec, t0
li t0, 0x90000000
csrw mtvec, t0
csrsi medeleg, 4
// Should trap to M mode
csrrw x1, mvendorid, t1