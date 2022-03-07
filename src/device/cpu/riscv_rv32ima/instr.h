#ifndef RISCV_RV32IMA_INSTR_H_
#define RISCV_RV32IMA_INSTR_H_

#include <stdint.h>

// all instructions are 32-bit and little endian
typedef uint32_t rv32i_instr_t;

#define opcode(i) (i & 0x1F)

// R-type instruction
typedef union{
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int func7 : 7;
    } instr;
} rv32i_r_instr_t;

// I-type instruction
typedef union {
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int imm : 12;
    } instr;
} rv32i_i_instr_t;

// S-type instruction
typedef union {
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int imm4_0 : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int imm11_5 : 7;
    } instr;
} rv32i_s_instr_t;

// B-type instruction
typedef union {
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int imm11 : 1;
        unsigned int imm4_1 : 4; 
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int imm10_5 : 6;
        unsigned int imm12 : 1;
    } instr;
} rv32i_b_instr_t;

// U-type instruction
typedef union {
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int imm : 20;
    } instr;
} rv32i_u_instr_t;

// J-type instruction
typedef union {
    uint32_t val;
    struct {
        unsigned int opcode : 7;
        unsigned int rd : 5;
        unsigned int imm19_12 : 8;
        unsigned int imm11 : 1;
        unsigned int imm10_1 : 10;
        unsigned int imm_20 : 1;
    } instr;
} rv32i_j_instr_t;

#define i_immediate(i) (((int32_t)i)>>20)
#define s_immediate(i) ((((int32_t)i)>>20 & ~0xFFFFFFE0) | (((uint32_t)i>>7) & 0x1F))
#define u_immediate(i) ((uint32_t)i & 0xFFFFF000)
// TODO: b and j immediates

#endif // RISCV_RV32IMA_INSTR_H_