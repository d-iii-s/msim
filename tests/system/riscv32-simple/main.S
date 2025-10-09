.text
lw a0, 0(x0)
lw a1, 4(x0)
add a2, a0, a1; # a2 = a0 + a1
sw a2, 8(x0)

.word 0x8C000073
