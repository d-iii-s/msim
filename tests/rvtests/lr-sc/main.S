.text
li a0, 0x94// var1 addr
addi a1, a0, 4 // var2 addr

// positive case
lr.w t0, (a0)
sc.w t0, zero, (a0)
bnez t0, fail
// sc after previous sc
sc.w t0, zero, (a0)
beqz t0, fail
// sc after sc to different addr
lr.w t0, (a0)
sc.w t0, zero, (a1)
sc.w t0, zero, (a0)
beqz t0, fail
// sc to wrong addr
lr.w t0, (a0)
sc.w t0, zero, (a1)
beqz t0, fail
// sc to last lr
lr.w t0, (a0)
lr.w t0, (a1)
sc.w t0, zero, (a1)
bnez t0, fail
// sc not to last lr
lr.w t0, (a0)
lr.w t0, (a1)
sc.w t0, zero, (a0)
beqz t0, fail
// store to different addr
lr.w t0, (a0)
sw zero, (a1)
sc.w t0, zero, (a0)
bnez t0, fail
// store to reserved addr
lr.w t0, (a0)
sw zero, (a0)
sc.w t0, zero, (a0)
beqz t0, fail
// unaligned store to reserved region
addi a2, a0, 2
lr.w t0, (a0)
sh zero, (a2)
sc.w t0, zero, (a0)
beqz t0, fail
.word 0x00200073
fail:
ebreak
var1:
    .word 1
var2:
    .word 2
