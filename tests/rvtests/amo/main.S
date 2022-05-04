.text
li a0, 0x54 // var1 addr
li a1, 5
amoswap.w t0, a1, (a0)
li a1, 3
amoadd.w t0, a1, (a0)
li a1, -1
amoxor.w t0, a1, (a0)
li a1, 15
amoand.w t0, a1, (a0)
li a1, 14
amoor.w t0, a1, (a0)
li a1, -2
amomin.w t0, a1, (a0)
li a1, -1
amomax.w t0, a1, (a0)
li a1, 0
amominu.w t0, a1, (a0)
li a1, -1
amomaxu.w t0, a1, (a0)
ebreak
.word 0x00200073
var1:
    .word 1
