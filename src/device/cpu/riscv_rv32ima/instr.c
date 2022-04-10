#include <stdio.h>
#include "instr.h"
#include "cpu.h"
#include "../../../assert.h"
#include "../../../main.h"

#include "instructions/computations.h"
#include "instructions/mem_ops.h"
#include "instructions/control_transfer.h"
#include "instructions/system.h"

static rv_exc_t illegal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // TODO: change state?
    return rv_exc_illegal_instruction;
}

static rv_instr_func_t decode_LOAD(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcLOAD);
    printf("LOAD instruction loaded");
    return load_instr; 
}

static rv_instr_func_t decode_MISC_MEM(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcMISC_MEM);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP_IMM(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcOP_IMM);
    return illegal_instr; 
}

static rv_instr_func_t decode_AUIPC(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcAUIPC);
    return illegal_instr; 
}

static rv_instr_func_t decode_STORE(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcSTORE);
    printf("STORE instruction");
    return store_instr;
}

static rv_instr_func_t decode_AMO(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcAMO);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcOP);
    printf("OP instruction");
    uint32_t funct = RV_R_FUNCT(instr);
    switch(funct){
        case rv_func_ADD:
            return add_instr;
        case rv_func_SUB:
        case rv_func_SLL:
        case rv_func_SLT:
        case rv_func_SLTU:
        case rv_func_XOR:
        case rv_func_SRL:
        case rv_func_SRA:
        case rv_func_OR:
        case rv_funcAND:
        default:
            return illegal_instr;
    }    
}

static rv_instr_func_t decode_LUI(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcLUI);
    return illegal_instr; 
}

static rv_instr_func_t decode_OP_32(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcOP_32);
    return illegal_instr; 
}

static rv_instr_func_t decode_BRANCH(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcBRANCH);
    return illegal_instr; 
}

static rv_instr_func_t decode_JALR(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcJALR);
    return illegal_instr; 
}

static rv_instr_func_t decode_JAL(rv_instr_t instr) {
    ASSERT(instr.r.opcode == rv_opcJAL);
    return illegal_instr; 
}


static rv_instr_func_t decode_PRIV(rv_instr_t instr){
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    ASSERT(instr.i.func3 == rv_funcPRIV);

    switch (instr.i.imm) {
    case rv_privEBREAK:
        return break_instr;
    case rv_privEHALT:
        return halt_instr;
    case rv_privECALL:
    default:
        return illegal_instr;
    }

}

static rv_instr_func_t decode_SYSTEM(rv_instr_t instr) {
    ASSERT(instr.i.opcode == rv_opcSYSTEM);
    switch (instr.i.func3) {
        case rv_funcPRIV:
            return decode_PRIV(instr);
        //TODO: add CSR instructions
        case rv_funcCSRRW:
        case rv_funcCSRRS:
        case rv_funcCSRRC:
        case rv_funcCSRRWI:
        case rv_funcCSRRSI:
        case rv_funcCSRRCI:
        default:
            return illegal_instr;
    }
}

rv_instr_func_t rv_instr_decode(rv_instr_t instr){
    // opcode is at the same spot in all encodings, so any can be chosen
    switch (instr.r.opcode) {
        case rv_opcLOAD: 
            return decode_LOAD(instr);
        case rv_opcMISC_MEM:
            return decode_MISC_MEM(instr);
        case rv_opcOP_IMM:
            return decode_OP_IMM(instr);
        case rv_opcAUIPC:
            return decode_AUIPC(instr);
        case rv_opcSTORE:
            return decode_STORE(instr);
        case rv_opcAMO:
            return decode_AMO(instr);
        case rv_opcOP:
            return decode_OP(instr);
        case rv_opcLUI:
            return decode_LUI(instr);
        case rv_opcOP_32:
            return decode_OP_32(instr);
        case rv_opcBRANCH:
            return decode_BRANCH(instr);
        case rv_opcJALR:
            return decode_JALR(instr);
        case rv_opcJAL:
            return decode_JAL(instr);
        case rv_opcSYSTEM: 
            return decode_SYSTEM(instr);
        default: {
            return illegal_instr;
        }
    }
}