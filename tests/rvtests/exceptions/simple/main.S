.text
li t0, 0x80000000
csrw mtvec, t0
csrrw x1, mvendorid, t1