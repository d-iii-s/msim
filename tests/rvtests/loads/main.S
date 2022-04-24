.text
// tests immediate operations
lb a0, 0x2C(x0)
lb a0, 0x30(x0)
lh a0, 0x2C(x0)
lh a0, 0x30(x0)
lw a0, 0x2C(x0)
lw a0, 0x30(x0)
lbu a0, 0x2C(x0)
lbu a0, 0x30(x0)
lhu a0, 0x2C(x0)
lhu a0, 0x30(x0)
.word 0x00200073

var1:
    .word 1
var2:
    .word -1
    