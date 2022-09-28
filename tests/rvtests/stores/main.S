#define ehalt .word 0x00200073
#define edump .word 0x00300073
.text
li a0, -1
li a1, 1
sb a0, 0(x0)
sb a1, 4(x0)
sh a0, 8(x0)
sh a1, 12(x0)
sw a0, 16(x0)
sw a1, 20(x0)
lw s0, 0(x0)
lw s1, 4(x0)
lw s2, 8(x0)
lw s3, 12(x0)
lw s4, 16(x0)
lw s5, 20(x0)
edump
ehalt