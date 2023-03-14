#define ehalt .word 0x8C000073
.text
li t0, 0x80000000
csrw mtvec, t0
csrrw x1, mvendorid, t1
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
ehalt