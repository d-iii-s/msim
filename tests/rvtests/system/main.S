#define ehalt .word 0x8C000073
#define edump .word 0x8C100073
#define etraces .word 0x8C200073
#define etracer .word 0x8C300073
#define ecsrd_t0 .word 0x8C4002F3
#define mie 0x304
.text
//ecall
//wfi
//sfence.vma
//sret
//mret
csrrw  zero, mtvec, t0
csrrs  zero, mtvec, t0
csrrc  zero, time, zero
csrrwi  zero, mtvec, 1
csrrsi  zero, mtvec, 31
csrrci  zero, mtvec, 0b11111
rdtime t0
ebreak
etraces
li t0, mie
ecsrd_t0
etracer
addi zero, zero, 0
edump
ehalt
