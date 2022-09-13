.text
li t0, 0x80000000
csrw mtvec, t0
li t0, 0xFFFFFFFF
csrw mie, t0
csrsi mip, 2
csrsi mstatus, 8
nop