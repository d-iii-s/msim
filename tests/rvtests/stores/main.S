.text
li a0, -1
li a1, 1
sb a0, 0x24(x0)
sb a1, 0x28(x0)
sh a0, 0x2C(x0)
sh a1, 0x30(x0)
sw a0, 0x34(x0)
sw a1, 0x38(x0)
ebreak

var1:
    .word 0
var2:
    .word 0
var3:
    .word 0
var4:
    .word 0
var5:
    .word 0
var6:
    .word 0