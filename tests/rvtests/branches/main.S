.text
/* Runs the positive test on branches */

BEQ_test:
    lw a0, 0x4C(x0) // 0
    lw a1, 0x4C(x0) // 0
    beq a0, a1, BNE_test
    EBREAK
BNE_test:
    lw a0, 0x50(x0) // 1
    bne a0, a1, BLT_test
    EBREAK
BLT_test:
    lw  a0, 0x54(x0) // -1
    blt a0, a1, BLTU_test
    EBREAK
BLTU_test:
    bltu a1, a0, BGE_test
    EBREAK
BGE_test:
    lw a0, 0x50(x0) // 1
    bge a0, a1, BGEU_test
    EBREAK
BGEU_test:
    lw  a0, 0x54(x0) // -1
    bgeu a0, a1, END
    EBREAK
END:
.word 0x00200073

var1:
    .word 0
var2:
    .word 1
var3:
    .word -1