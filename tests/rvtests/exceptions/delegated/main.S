.text
li t0, 0x80000000
csrw stvec, t0
csrsi medeleg, 4
li t1, 0x18
csrw mepc, t1
// Returns to U-mode
mret
csrrw x1, mvendorid, t1
