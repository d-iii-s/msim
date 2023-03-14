#define ehalt .word 0x8C000073
.text
li a0, 0
addi a1, a0, 4 
li s0, 0x90000000
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
li s1, 'S'
sw s1, (s0)
ehalt
fail:
li s1, 'F'
sw s1, (s0)
ehalt