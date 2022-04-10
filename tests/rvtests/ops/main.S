.text
/* Read vals from mem, add them together and store to mem */
lw a0, 0x38(x0)
lw a1, 0x3C(x0)
lw a2, 0x40(x0)
add t0, a0, a1
sub t0, a0, a1
sll t0, a0, a1
slt t0, a2, a0
sltu t0, a2, a0
sra t0, a2, a1
srl t0, a2, a1
xor t0, a2, a1
or t0, a0, a1
and t0, a1, a2
.word 0x00200073

var1:
    .word 1
var2:
    .word 2
var3:
    .word -1