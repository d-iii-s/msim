#include "instr.h"
#include "cpu.h"
#include "../../assert.h"

static rv_exc_t illegal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // TODO: change state?
    return rv_exc_illegal_instruction;
}

static rv_instr_func_t decode_LOAD(rv_instr_t instr) {
    ASSERT(instr.r.opcode == LOAD);
    return illegal_instr; 
}

static rv_instr_func_t decode_MISC_MEM(rv_instr_t instr) {
    ASSERT(instr.r.opcode == MISC_MEM);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP_IMM(rv_instr_t instr) {
    ASSERT(instr.r.opcode == OP_IMM);
    return illegal_instr; 
}

static rv_instr_func_t decode_AUIPC(rv_instr_t instr) {
    ASSERT(instr.r.opcode == AUIPC);
    return illegal_instr; 
}

static rv_instr_func_t decode_STORE(rv_instr_t instr) {
    ASSERT(instr.r.opcode == STORE);
    return illegal_instr; 
}

static rv_instr_func_t decode_AMO(rv_instr_t instr) {
    ASSERT(instr.r.opcode == AMO);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP(rv_instr_t instr) {
    ASSERT(instr.r.opcode == OP);
    return illegal_instr; 
}

static rv_instr_func_t decode_LUI(rv_instr_t instr) {
    ASSERT(instr.r.opcode == LUI);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP_32(rv_instr_t instr) {
    ASSERT(instr.r.opcode == OP_32);
    return illegal_instr; 
}

static rv_instr_func_t decode_BRANCH(rv_instr_t instr) {
    ASSERT(instr.r.opcode == BRANCH);
    return illegal_instr; 
}

static rv_instr_func_t decode_JALR(rv_instr_t instr) {
    ASSERT(instr.r.opcode == JALR);
    return illegal_instr; 
}

static rv_instr_func_t decode_JAL(rv_instr_t instr) {
    ASSERT(instr.r.opcode == JAL);
    return illegal_instr; 
}

static rv_instr_func_t decode_SYSTEM(rv_instr_t instr) {
    ASSERT(instr.r.opcode == SYSTEM);
    return illegal_instr; 
}

rv_instr_func_t rv_instr_decode(rv_instr_t instr){
    // opcode is at the same spot in all encodings, so any can be chosen
    switch (instr.r.opcode) {
        case LOAD: 
            return decode_LOAD(instr);
        case MISC_MEM:
            return decode_MISC_MEM(instr);
        case OP_IMM:
            return decode_OP_IMM(instr);
        case AUIPC:
            return decode_AUIPC(instr);
        case STORE:
            return decode_STORE(instr);
        case AMO:
            return decode_AMO(instr);
        case OP:
            return decode_OP(instr);
        case LUI:
            return decode_LUI(instr);
        case OP_32:
            return decode_OP_32(instr);
        case BRANCH:
            return decode_BRANCH(instr);
        case JALR:
            return decode_JALR(instr);
        case JAL:
            return decode_JAL(instr);
        case SYSTEM: 
            return decode_SYSTEM(instr);
        default: {
            return illegal_instr;
        }
    }
}