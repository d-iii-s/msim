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
        int imm : 12;
    } i;
    struct {
        unsigned int opcode : 7;
        unsigned int imm4_0 : 5;
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        int imm11_5 : 7;
    } s;
    struct {
        unsigned int opcode : 7;
        unsigned int imm11 : 1;
        unsigned int imm4_1 : 4; 
        unsigned int func3 : 3;
        unsigned int rs1 : 5;
        unsigned int rs2 : 5;
        unsigned int imm10_5 : 6;
        int imm12 : 1;
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
        int imm20 : 1;
    } j;
} rv_instr_t;

static_assert(sizeof(rv_instr_t) == 4, "rv_instr_t has wrong size");

#define RV_S_IMM(instr) (uint32_t)((((int32_t)instr.s.imm11_5)<<5)|((0x1F)&instr.s.imm4_0))
#define RV_R_FUNCT(instr) (uint32_t)(((uint32_t)(instr.r.func7)<<3)|(0x7 & instr.r.func3))
#define RV_J_IMM(instr) (uint32_t)((((int32_t)instr.j.imm20)<<20)|(instr.j.imm19_12<<12)|(instr.j.imm11<<11)|(instr.j.imm10_1 << 1))
#define RV_B_IMM(instr) (uint32_t)((((int32_t)instr.b.imm12)<<12)|(instr.b.imm11<<11)|(instr.b.imm10_5<<5)|(instr.b.imm4_1<<1))

/** Opcodes*/
typedef enum {
    rv_opcLOAD        = 0b0000011,
    rv_opcLOAD_FP     = 0b0000111, // not supported
    rv_opcMISC_MEM    = 0b0001111,
    rv_opcOP_IMM      = 0b0010011,
    rv_opcAUIPC       = 0b0010111,
    rv_opcSTORE       = 0b0100011,
    rv_opcSTORE_FP    = 0b0100111, // not supported
    rv_opcAMO         = 0b0101111,
    rv_opcOP          = 0b0110011,
    rv_opcLUI         = 0b0110111,
    rv_opcOP_32       = 0b0111011,
    rv_opcMADD        = 0b1000011, // not supported
    rv_opcMSUB        = 0b1000111, // not supported
    rv_opcNMSUB       = 0b1001011, // not supported
    rv_opcNMADD       = 0b1001111, // not supported
    rv_opcOP_FP       = 0b1010011, // not supported
    rv_opcBRANCH      = 0b1100011,
    rv_opcJALR        = 0b1100111,
    rv_opcJAL         = 0b1101111,
    rv_opcSYSTEM      = 0b1110011
} rv_opcode_t;

/** Funct values for OP instructions */
typedef enum {
    rv_func_ADD       = 0b0000000000,
    rv_func_SUB       = 0b0100000000,
    rv_func_SLL       = 0b0000000001,
    rv_func_SLT       = 0b0000000010,
    rv_func_SLTU      = 0b0000000011,
    rv_func_XOR       = 0b0000000100,
    rv_func_SRL       = 0b0000000101,
    rv_func_SRA       = 0b0100000101,
    rv_func_OR        = 0b0000000110,
    rv_func_AND       = 0b0000000111
} rv_op_func_t;

/** Funct values for OP-imm instructions */
typedef enum {
    rv_func_ADDI      = 0b000,
    rv_func_SLTI      = 0b010,
    rv_func_SLTIU     = 0b011,
    rv_func_XORI      = 0b100,
    rv_func_ORI       = 0b110,
    rv_func_ANDI      = 0b111,
    rv_func_SLLI      = 0b001,
    rv_func_SRI       = 0b101, // same for SRLI and SRAI
} rv_op_imm_func_t;

// bit that is set if the right shift by constant is arithmetical or logical
// this bit position is in the imm field, not in the whole instruction
#define RV_IMM_SHIFT_ARITHMETIC_BIT (1<<10)

// mask for the shift ammount in the imm field of the instruction
#define RV_IMM_SHIFT_SHAMT_MASK (0x1F)

/** Funct values for BRANCH instructions */
typedef enum {
    rv_func_BEQ       = 0b000,
    rv_func_BNE       = 0b001,
    rv_func_BLT       = 0b100,
    rv_func_BLTU      = 0b110,
    rv_func_BGE       = 0b101,
    rv_func_BGEU      = 0b111
} rv_branch_func_t;

/** Funct values for SYSTEM instructions */
typedef enum {
    rv_funcPRIV       = 0b000,
    rv_funcCSRRW      = 0b001,
    rv_funcCSRRS      = 0b010,
    rv_funcCSRRC      = 0b011,
    rv_funcCSRRWI     = 0b101,
    rv_funcCSRRSI     = 0b110,
    rv_funcCSRRCI     = 0b111,

} rv_system_func_t;

/** Immediate values for PRIV SYSTEM instructions */
typedef enum {
    rv_privECALL  = 0,
    rv_privEBREAK = 1,
    rv_privEHALT  = 2
} rv_system_priv_imm_t;


//forward declarations
enum rv_exc;
struct rv_cpu;

typedef enum rv_exc (*rv_instr_func_t)(struct rv_cpu*, rv_instr_t);

extern rv_instr_func_t rv_instr_decode(rv_instr_t instr);

extern enum rv_exc illegal_instr(struct rv_cpu *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_H_