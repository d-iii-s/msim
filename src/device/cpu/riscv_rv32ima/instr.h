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
} rv_instr_t;

static_assert(sizeof(rv_instr_t) == 4, "rv_instr_t has wrong size");

typedef enum {
    LOAD        = 0b0000011,
    LOAD_FP     = 0b0000111, // not supported
    MISC_MEM    = 0b0001111,
    OP_IMM      = 0b0010011,
    AUIPC       = 0b0010111,
    STORE       = 0b0100011,
    STORE_FP    = 0b0100111, // not supported
    AMO         = 0b0101111,
    OP          = 0b0110011,
    LUI         = 0b0110111,
    OP_32       = 0b0111011,
    MADD        = 0b1000011, // not supported
    MSUB        = 0b1000111, // not supported
    NMSUB       = 0b1001011, // not supported
    NMADD       = 0b1001111, // not supported
    OP_FP       = 0b1010011, // not supported
    BRANCH      = 0b1100011,
    JALR        = 0b1100111,
    JAL         = 0b1101111,
    SYSTEM      = 0b1110011
} rv_opcode_t;


//forward declarations
enum rv_exc;
struct rv_cpu;

typedef enum rv_exc (*rv_instr_func_t)(struct rv_cpu*, rv_instr_t);

extern rv_instr_func_t rv_instr_decode(rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_H_