.text
ecall
sfence.vma
sret
mret
wfi
csrrw  zero, mtvec, t0
csrrs  zero, mtvec, t0
csrrc  zero, time, zero
csrrwi  zero, mtvec, 1
csrrsi  zero, mtvec, 31
csrrci  zero, mtvec, 0b11111
rdtime t0
.word 0x00200073