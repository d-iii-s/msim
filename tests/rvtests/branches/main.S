#define ehalt .word 0x8C000073
.text
li t0, 0x90000000

BEQ_pos_test:
    li a0, 0
    li a1, 0
    // 0 == 0
    beq a0, a1, BEQ_neg_test
    j ERROR
BEQ_neg_test:
    li a0, 1
    // 0 == 1
    beq a0, a1, ERROR
BNE_pos_test:
    // 0 != 1
    bne a0, a1, BNE_neg_test
    j ERROR
BNE_neg_test:
    li a0, 0
    // 0 != 0
    bne a0, a1, ERROR
BLT_pos_test:
    li  a0, -1
    // -1 < 1
    blt a0, a1, BLT_neg_test
    j ERROR
BLT_neg_test:
    li  a0, 1
    // 1 < 1
    blt a0, a1, ERROR
BLTU_pos_test:
    li  a0, -1
    // 1 < -1 (unsigned)
    bltu a1, a0, BLTU_neg_test
    j ERROR
BLTU_neg_test:
    li a0, -1
    // -1 < 1 (unsigned)
    bltu a0, a1, ERROR
BGE_pos_test:
    li a0, -1
    // 1 >= -1
    bge a1, a0, BGE_eq_test
    j ERROR
BGE_eq_test:
    li a0, 1
    bge a0, a1, BGEU_pos_test
    j ERROR
BGE_neg_test:
    li a0, -1
    // -1 >= 1
    bge a0, a1, ERROR
BGEU_pos_test:
    li  a0, -1
    // -1 >= 1 (unsigned)
    bgeu a0, a1, BGEU_eq_test
    j ERROR
BGEU_eq_test:
    li a0, 1
    // 1 == 1
    bgeu a0, a1, BGEU_neg_test
    j ERROR
BGEU_neg_test:
    li a0, -1
    // 1 >= -1 (unsigned)
    bgeu a1, a0, ERROR
    j END
END:
li t1, 'S'
sw t1, 0(t0)
ehalt
ERROR:
li t1, 'F'
sw t1, 0(t0)
ehalt
