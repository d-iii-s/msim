.text
li a0, 0x1C // var1 addr
li a1, 0
lr.w t0, (a0)
sc.w t0, a1, (a0)
bnez t0, fail
.word 0x00200073
fail:
ebreak
var1:
    .word 1
