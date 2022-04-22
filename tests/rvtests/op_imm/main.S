.text
// tests immediate operations
lw a0, 0x30(x0)
lw a1, 0x34(x0)
addi a2, a0, 1
slti a2, a1, 0
sltiu a2, a1, 0
andi a2, a0, 3
ori a2, a0, 2
xori a2, a0, 3
slli a2, a0, 1
srli a2, a1, 1
srai a2, a1, 1
.word 0x00200073

var1:
    .word 1
var2:
    .word -1