#define ehalt .word 0x8C000073
#define edump .word 0x8C100073
.text
// tests immediate operations
li a0, 1
li a1, -2
addi s0, a0, -1
slti s1, a1, -1
sltiu s2, a1, 0
andi s3, a0, 3
ori s4, a0, 2
xori s5, a0, -1
slli s6, a0, 1
srli s7, a1, 1
srai s8, a1, 1
lui s9, 0x80001
auipc s10, 0x80001
edump
ehalt