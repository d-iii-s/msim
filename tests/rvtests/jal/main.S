.text
/* Performs a simple loop indefinetly */
lw a0, 0x14(x0)
lw a1, 0x18(x0)
loop:
add a0, a0, a1
j loop
.word 0x00200073

var1:
    .word 0
var2:
    .word 1