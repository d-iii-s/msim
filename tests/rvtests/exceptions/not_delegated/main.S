.text
li t0, 0x80000000
csrw stvec, t0
li t0, 0x90000000
csrw mtvec, t0
li t1, 0x1C
csrw mepc, t1
// Returns to U-mode
mret
csrrw x1, mvendorid, t1
