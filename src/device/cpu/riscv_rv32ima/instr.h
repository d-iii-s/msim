#ifndef RISCV_RV32IMA_INSTR_H_
#define RISCV_RV32IMA_INSTR_H_

#include <stdint.h>
#include <assert.h>

typedef union{
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int func7 : 7;
    } r;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int imm : 12;
    } i;
    struct {
        unsigned int opcode : 7;
        unsigned int imm4_0 : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int imm11_5 : 7;
    } s;
    struct {
        unsigned int opcode : 7;
        unsigned int imm11 : 1;
        unsigned int imm4_1 : 4; 
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int imm10_5 : 6;
        unsigned int imm12 : 1;
    } b;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int imm : 20;
    } u;
        struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int imm19_12 : 8;
        unsigned int imm11 : 1;
        unsigned int imm10_1 : 10;
        unsigned int imm_20 : 1;
    } j;
} rv32i_instr_t;

static_assert(sizeof(rv32i_instr_t) == 4, "rv32i_instr_t has wrong size");

#endif // RISCV_RV32IMA_INSTR_H_