.text
li t0, 0x800FFFD0
csrw satp, t0
li t0, 1 << 11
csrs mstatus, t0
auipc t0, 0
addi t0, t0, 16
csrw mepc, t0
mret
lw a1, 0(zero)
li t0, 0x1000
lw a2, 0(t0)
ebreak