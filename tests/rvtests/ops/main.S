#define ehalt .word 0x00200073
#define edump .word 0x00300073
.text
/* Read vals from mem, tests all inter register ops and halts */
li a0, 1
li a1, 2
li a2, -1
// 1 + 2 = 3
add s0, a0, a1
// 1 - 2 = -1
sub s1, a0, a1
// 1 << 2 = 4
sll s2, a0, a1
// -1 < 1 ?
slt s3, a2, a0
// 2^32 - 1 < 1 ?
sltu s4, a2, a0
// -1 >> 2 (sign extended)
sra s5, a2, a1
// -1 >> 2 (zero extended)
srl s6, a2, a1
// -1 ^ 2 = -3 ?
xor s7, a2, a1
// 1 | 2 = 3
or s8, a0, a1
// 2 & -1 = 2
and s9, a1, a2
edump
ehalt