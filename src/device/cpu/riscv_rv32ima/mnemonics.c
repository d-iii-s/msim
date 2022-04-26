#include "mnemonics.h"

void undefined_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "(undefined)");
}

extern rv_mnemonics_func_t rv_decode_mnemonics(rv_instr_t instr){
    return undefined_mnemonics;
}

