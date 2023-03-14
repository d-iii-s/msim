#define ehalt .word 0x8C000073
.text
li a0, 0 // address on which the instructions will be executed. First value is 0
li s0, 0x90000000

li a1, 5
amoswap.w t0, a1, (a0)
bnez t0, error
mv a2, a1
li a1, 3
amoadd.w t0, a1, (a0)
bne t0, a2, error
add a2, a2, a1
li a1, -1
amoxor.w t0, a1, (a0)
bne t0, a2, error
xor a2, a2, a1
li a1, 15
amoand.w t0, a1, (a0)
bne t0, a2, error
and a2, a2, a1
li a1, 14
amoor.w t0, a1, (a0)
bne t0, a2, error
or a2, a2, a1
li a1, -2
amomin.w t0, a1, (a0)
bne t0, a2, error
mv a2, a1
li a1, -1
amomax.w t0, a1, (a0)
bne t0, a2, error
mv a2, a1
li a1, 0
amominu.w t0, a1, (a0)
bne t0, a2, error
mv a2, a1
li a1, -1
amomaxu.w t0, a1, (a0)
bne t0, a2, error
li t0, 'S'
sw t0, (s0)
ehalt
error:
    li t0, 'F'
    sw t0, (s0)
    ehalt
