#ifndef RISCV_RV32IMA_MNEMONICS_H_
#define RISCV_RV32IMA_MNEMONICS_H_

#include <stdint.h>

#include "cpu.h"
#include "instr.h"

// Dissasembly mnemonics
typedef void (*rv_mnemonics_func_t)(uint32_t, rv_instr_t, string_t *, string_t *);

extern rv_mnemonics_func_t rv_decode_mnemonics(rv_instr_t instr);

extern void undefined_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);



#endif // RISCV_RV32IMA_MNEMONICS_H_