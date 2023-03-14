#define ehalt .word 0x8C000073
.text
li t0, 0x80000000
csrw stvec, t0
li t0, 0x70000000
csrw mtvec, t0
auipc t1, 0
addi t1, t1, 16
csrw mepc, t1
// Returns to U-mode
mret
csrrw x1, sepc, t1
li t1, 'F'
sw t1, (t0)
ehalt
