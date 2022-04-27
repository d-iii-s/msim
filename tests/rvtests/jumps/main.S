.text
lw a0, 0x1C(x0)
lw a1, 0x20(x0)
j after
ebreak
after:
jalr zero, 0x18(a0)
ebreak
.word 0x00200073

var1:
    .word 0
var2:
    .word 1