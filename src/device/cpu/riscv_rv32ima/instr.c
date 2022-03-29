#include "instr.h"
#include "cpu.h"

static rv_exc_t illegal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // TODO: change state?
    return rv_exc_illegal_instruction;
}


rv_instr_func_t rv_instr_decode(rv_instr_t intr){
    return illegal_instr;
}