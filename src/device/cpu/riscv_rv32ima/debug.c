#include "debug.h"
#include "cpu.h"
#include "../../../assert.h"
#include "../../../fault.h"

char *rv_reg_name_table[__rv_regname_type_count][RV_REG_COUNT] = {
    {
         "x0",  "x1",  "x2",  "x2",  "x4",  "x5",  "x6",  "x7",
         "x8",  "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
    },
    {
         "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
        "s0/fp", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
           "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
           "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    }
};

char** rv_regnames;

static rv_regname_type_t curr_regname_type = rv_regname_abi;

void rv_debug_init(void){
    rv_regnames = rv_reg_name_table[curr_regname_type];
}

bool rv_debug_change_regnames(unsigned int type){
    if(type >= __rv_regname_type_count){
        error("Index out of range 0..%u", __rv_regname_type_count - 1);
        return false;
    }
    curr_regname_type = (rv_regname_type_t)type;
    rv_regnames = rv_reg_name_table[curr_regname_type];
    return true;
}

void rv_reg_dump(rv_cpu_t *cpu){
    
}

void rv_idump(rv_cpu_t *cpu, uint32_t addr, rv_instr_t instr, bool modregs){

}
