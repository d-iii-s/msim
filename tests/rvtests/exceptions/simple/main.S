.text
li t0, 0x80000000
csrw mtvec, t0
li t0, 0xFFFFFFFF & ~0b100
csrrs zero, medeleg, t0
li t0, 0xFFFFFFFF
csrrs zero, mideleg, t0
li t0, 0x80600001
csrw satp, t0
csrrw x1, mvendorid, t1