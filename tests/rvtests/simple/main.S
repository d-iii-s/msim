.text
/* Read vals from mem, add them together and store to mem */
lw a0, 0x14(x0)
lw a1, 0x18(x0)
add a2, a0, a1; # a2 = a0 + a1
sw a2, 0x1C(x0)
.word 0x00200073

var1:
    .word 1
var2:
    .word 2
var3:
    .word 0