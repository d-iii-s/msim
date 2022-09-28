#define ehalt .word 0x00200073
#define edump .word 0x00300073

.text
lb a0, 0(zero)
edump
lb a0, 4(zero)
edump
lh a0, 0(zero)
edump
lh a0, 4(zero)
edump
lw a0, 0(zero)
edump
lw a0, 4(zero)
edump
lbu a0, 0(zero)
edump
lbu a0, 4(zero)
edump
lhu a0, 0(zero)
edump
lhu a0, 4(zero)
edump
ehalt