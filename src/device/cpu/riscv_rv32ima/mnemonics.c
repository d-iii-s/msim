/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V debugging information formatting
 *
 */

#include <stdint.h>
#include <sys/time.h>

#include "mnemonics.h"
#include "debug.h"
#include "instr.h"
#include "instructions/computations.h"
#include "instructions/control_transfer.h"
#include "instructions/system.h"
#include "instructions/mem_ops.h"
#include "../../../env.h"
#include "../../../assert.h"
#include "../../../utils.h"

extern rv_mnemonics_func_t rv_decode_mnemonics(rv_instr_t instr){
    // is this dirty?
    rv_instr_func_t instr_func = rv_instr_decode(instr);

    // very dirty indeed, but only one decode is needed

    #define IF_SAME_DECODE(expected_instr) if(instr_func == expected_instr ## _instr) return rv_ ## expected_instr ## _mnemonics

    IF_SAME_DECODE(lui);
    IF_SAME_DECODE(auipc);

    IF_SAME_DECODE(jal);
    IF_SAME_DECODE(jalr);
    IF_SAME_DECODE(beq);
    IF_SAME_DECODE(bne);
    IF_SAME_DECODE(blt);
    IF_SAME_DECODE(bge);
    IF_SAME_DECODE(bltu);
    IF_SAME_DECODE(bgeu);

    IF_SAME_DECODE(lb);
    IF_SAME_DECODE(lh);
    IF_SAME_DECODE(lw);
    IF_SAME_DECODE(lbu);
    IF_SAME_DECODE(lhu);

    IF_SAME_DECODE(sb);
    IF_SAME_DECODE(sh);
    IF_SAME_DECODE(sw);

    IF_SAME_DECODE(addi);
    IF_SAME_DECODE(slti);
    IF_SAME_DECODE(sltiu);
    IF_SAME_DECODE(xori);
    IF_SAME_DECODE(ori);
    IF_SAME_DECODE(andi);
    IF_SAME_DECODE(slli);
    IF_SAME_DECODE(srli);
    IF_SAME_DECODE(srai);

    IF_SAME_DECODE(add);
    IF_SAME_DECODE(sub);
    IF_SAME_DECODE(sll);
    IF_SAME_DECODE(slt);
    IF_SAME_DECODE(sltu);
    IF_SAME_DECODE(xor);
    IF_SAME_DECODE(or);
    IF_SAME_DECODE(and);
    IF_SAME_DECODE(srl);
    IF_SAME_DECODE(sra);

    IF_SAME_DECODE(fence);

    // SYSTEM 

    if(instr_func == break_instr)
        return rv_ebreak_mnemonics;
    
    if(instr_func == halt_instr)
        return rv_ehalt_mnemonics;

	
    if(instr_func == dump_instr)
        return rv_edump_mnemonics;

    if(instr_func == call_instr)
        return rv_ecall_mnemonics;

    IF_SAME_DECODE(sret);
    IF_SAME_DECODE(mret);
    IF_SAME_DECODE(wfi);
    IF_SAME_DECODE(sfence);

    IF_SAME_DECODE(csrrw);
    IF_SAME_DECODE(csrrs);
    IF_SAME_DECODE(csrrc);
    IF_SAME_DECODE(csrrwi);
    IF_SAME_DECODE(csrrsi);
    IF_SAME_DECODE(csrrci);

    // M-extension

    IF_SAME_DECODE(mul);
    IF_SAME_DECODE(mulh);
    IF_SAME_DECODE(mulhsu);
    IF_SAME_DECODE(mulhu);
    IF_SAME_DECODE(div);
    IF_SAME_DECODE(divu);
    IF_SAME_DECODE(rem);
    IF_SAME_DECODE(remu);

    // A-extension

    IF_SAME_DECODE(lr);
    IF_SAME_DECODE(sc);
    IF_SAME_DECODE(amoswap);
    IF_SAME_DECODE(amoadd);
    IF_SAME_DECODE(amoxor);
    IF_SAME_DECODE(amoor);
    IF_SAME_DECODE(amoand);
    IF_SAME_DECODE(amomin);
    IF_SAME_DECODE(amomax);
    IF_SAME_DECODE(amominu);
    IF_SAME_DECODE(amomaxu);

    return undefined_mnemonics;
}

/***********
 * Helpers *
 ***********/

static void r_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    string_printf(s_mnemonics, " %s, %s, %s",
        rv_regnames[instr.r.rd],
        rv_regnames[instr.r.rs1],
        rv_regnames[instr.r.rs2]
    );
}

static void r_instr_comment_binop(rv_instr_t instr, string_t *s_comments, const char *op) {
    string_printf(s_comments, "%s = %s %s %s",
        rv_regnames[instr.r.rd],
        rv_regnames[instr.r.rs1],
        op,
        rv_regnames[instr.r.rs2]
    );
}

static void i_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    string_printf(s_mnemonics, " %s, %s, %d",
        rv_regnames[instr.i.rd],
        rv_regnames[instr.i.rs1],
        instr.i.imm
    );
}

static void i_instr_comment_binop(rv_instr_t instr, string_t *s_comments, const char *op) {
    string_printf(s_comments, "%s = %s %s %d",
        rv_regnames[instr.i.rd],
        rv_regnames[instr.i.rs1],
        op,
        instr.i.imm
    );
}

static void i_instr_comment_binop_unsigned(rv_instr_t instr, string_t *s_comments, const char *op) {
    string_printf(s_comments, "%s = %s %s %u",
        rv_regnames[instr.i.rd],
        rv_regnames[instr.i.rs1],
        op,
        instr.i.imm
    );
}

static void i_instr_comment_binop_hex(rv_instr_t instr, string_t *s_comments, const char *op) {
    string_printf(s_comments, "%s = %s %s 0x%08x",
        rv_regnames[instr.i.rd],
        rv_regnames[instr.i.rs1],
        op,
        instr.i.imm
    );
}

static void imm_shift_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    int32_t shamt = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;
    string_printf(s_mnemonics, " %s, %s, %u", rv_regnames[instr.i.rd], rv_regnames[instr.i.rs1], shamt);
}

static void imm_shift_comments(rv_instr_t instr, string_t *s_comments, const char* op){
    int32_t shamt = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;
    string_printf(s_comments, "%s = %s %s %u", rv_regnames[instr.i.rd], rv_regnames[instr.i.rs1], op, shamt);
}

static void dissasemble_target(int reg, int32_t offset, string_t *s_mnemonics){
    if(reg >= 0 && reg < RV_REG_COUNT){
        string_printf(s_mnemonics, "%d(%s)", offset, rv_regnames[reg]);
    }
    else {
        string_printf(s_mnemonics, "%d", offset);
    }
}

static void load_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){

    int32_t imm = instr.i.imm;

    string_printf(s_mnemonics, " %s, ", rv_regnames[instr.i.rd]);

    dissasemble_target(instr.i.rs1, imm, s_mnemonics);
}

static void store_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){

    int32_t imm = RV_S_IMM(instr);

    string_printf(s_mnemonics, " %s, ", rv_regnames[instr.s.rs2]);
    dissasemble_target(instr.s.rs1, imm, s_mnemonics);
}

static void j_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    int32_t imm = RV_J_IMM(instr);

    string_printf(s_mnemonics, " %s, ", rv_regnames[instr.j.rd]);

    dissasemble_target(-1, imm, s_mnemonics);
}

static void j_instr_comments(rv_instr_t instr, uint32_t addr, string_t *s_comments){
    int32_t imm = RV_J_IMM(instr);
    string_printf(s_comments, "target: %#010x", addr + imm);
}

static void jalr_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    int32_t imm = instr.i.imm;
    string_printf(s_mnemonics, " %s, ", rv_regnames[instr.i.rd]);
    dissasemble_target(instr.i.rs1, imm, s_mnemonics);
}

static void b_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    int32_t imm = RV_B_IMM(instr);
    string_printf(s_mnemonics, " %s, %s, ", rv_regnames[instr.b.rs1], rv_regnames[instr.b.rs2]);
    dissasemble_target(-1, imm, s_mnemonics);
}

static void b_instr_comments(rv_instr_t instr, uint32_t addr, string_t *s_comments, const char* op){
    int32_t imm = RV_B_IMM(instr);
    string_printf(s_comments, "branch to %#010x if %s %s %s", addr+imm, rv_regnames[instr.b.rs1], op, rv_regnames[instr.b.rs2]);
}

static void u_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics){
    string_printf(s_mnemonics, " %s, %#07x", rv_regnames[instr.u.rd], instr.u.imm);
}

static void amo_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics) {
    string_printf(s_mnemonics, " %s, %s, (%s)", rv_regnames[instr.r.rd], rv_regnames[instr.r.rs2], rv_regnames[instr.r.rs1]);
}

static void csr_reg_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics) {
    string_printf(s_mnemonics, " %s, %s, %s", rv_regnames[instr.i.rd], rv_csrnames[RV_I_UNSIGNED_IMM(instr)], rv_regnames[instr.i.rs1]);
}

static void csr_imm_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics) {
    string_printf(s_mnemonics, " %s, %s, %u", rv_regnames[instr.i.rd], rv_csrnames[RV_I_UNSIGNED_IMM(instr)], instr.i.rs1 & 0x1F);
}

/***********************************
 * Mnemonics/Dissasembly functions *
 ***********************************/

void undefined_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "(undefined)");
}

// relative addressing
extern void rv_lui_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lui");
    u_instr_mnemonics(instr, s_mnemonics);

    string_printf(s_comments, "%s = 0x%08x", rv_regnames[instr.u.rd], instr.u.imm << 12);
}
extern void rv_auipc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "auipc");
    u_instr_mnemonics(instr, s_mnemonics);

    uint32_t imm = instr.u.imm << 12;

    string_printf(s_comments, "%s = pc + 0x%08x (= 0x%08x)", rv_regnames[instr.u.rd], imm, imm + addr);
}

// control transfer
extern void rv_jal_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "jal");
    j_instr_mnemonics(instr, s_mnemonics);
    j_instr_comments(instr, addr, s_comments);
}
extern void rv_jalr_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "jalr");
    jalr_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_beq_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "beq");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, "==");
}
extern void rv_bne_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "bne");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, "!=");
}
extern void rv_blt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "blt");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, "<");
    string_printf(s_comments, " (signed)");
}
extern void rv_bge_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "bge");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, ">=");
    string_printf(s_comments, " (signed)");
}
extern void rv_bltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "bltu");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, "<");
    string_printf(s_comments, " (unsigned)");
}
extern void rv_bgeu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "bgeu");
    b_instr_mnemonics(instr, s_mnemonics);
    b_instr_comments(instr, addr, s_comments, ">=");
    string_printf(s_comments, " (unsigned)");
}

// mem load
extern void rv_lb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lb");
    load_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_lh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lh");
    load_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_lw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lw");
    load_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_lbu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lbu");
    load_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_lhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lhu");
    load_instr_mnemonics(instr, s_mnemonics);
}

// mem store
extern void rv_sb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sb");
    store_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_sh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sh");
    store_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_sw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sw");
    store_instr_mnemonics(instr, s_mnemonics);
}

// op imm
extern void rv_addi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "addi");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop(instr, s_comments, "+");
}
extern void rv_slti_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "slti");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop(instr, s_comments, "<");
    string_printf(s_comments, " (signed)");
}
extern void rv_sltiu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sltiu");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop_unsigned(instr, s_comments, "<");
    string_printf(s_comments, " (unsigned)");
}
extern void rv_xori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "xori");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop_hex(instr, s_comments, "^");
}
extern void rv_ori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "ori");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop_hex(instr, s_comments, "|");
}
extern void rv_andi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "andi");
    i_instr_mnemonics(instr, s_mnemonics);
    i_instr_comment_binop_hex(instr, s_comments, "&");
}
extern void rv_slli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "slli");
    imm_shift_mnemonics(instr, s_mnemonics);
    imm_shift_comments(instr, s_comments, "<<");
}
extern void rv_srli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "srli");
    imm_shift_mnemonics(instr, s_mnemonics);
    imm_shift_comments(instr, s_comments, ">>");
    string_printf(s_comments, " (logical)");
}
extern void rv_srai_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "srai");
    imm_shift_mnemonics(instr, s_mnemonics);
    imm_shift_comments(instr, s_comments, ">>");
    string_printf(s_comments, " (arithmetical)");
}

// reg op
extern void rv_add_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "add");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "+");
}
extern void rv_sub_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sub");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "-");
}
extern void rv_sll_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sll");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "<<");
}
extern void rv_slt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "slt");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "<");
    string_printf(s_comments, " (signed)");
}
extern void rv_sltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sltu");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "<");
    string_printf(s_comments, " (unsigned)");
}
extern void rv_xor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "xor");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "^");
}
extern void rv_srl_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "srl");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, ">>");
    string_printf(s_comments, " (logical)");
}
extern void rv_sra_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sra");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, ">>");
    string_printf(s_comments, " (arithmetical)");
}
extern void rv_or_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "or");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "|");
}
extern void rv_and_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "and");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "&");
}

// mem misc
extern void rv_fence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "fence");
}

// system
extern void rv_ecall_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "ecall");
}
extern void rv_ebreak_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "ebreak");
}
extern void rv_ehalt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "ehalt");
}

void rv_edump_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
	string_printf(s_mnemonics, "edump");
}

extern void rv_sret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sret");
}

extern void rv_mret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "mret");
}

extern void rv_wfi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "wfi");
}
extern void rv_sfence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
   string_printf(s_mnemonics, "sfence.vma"); 
}

// CSR
extern void rv_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrw");
    csr_reg_instr_mnemonics(instr, s_mnemonics);

    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    char* rs1 = rv_regnames[instr.i.rs1];
    string_printf(s_comments, "%s = %s, %s = %s", rd, csr, csr, rs1);
}
extern void rv_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrs");
    csr_reg_instr_mnemonics(instr, s_mnemonics);

    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    char* rs1 = rv_regnames[instr.i.rs1];
    string_printf(s_comments, "%s = %s, %s |= %s", rd, csr, csr, rs1);
}
extern void rv_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrc");
    csr_reg_instr_mnemonics(instr, s_mnemonics);

    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    char* rs1 = rv_regnames[instr.i.rs1];
    string_printf(s_comments, "%s = %s, %s &= ~%s", rd, csr, csr, rs1);
}
extern void rv_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrwi");
    csr_imm_instr_mnemonics(instr, s_mnemonics);

    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    uint32_t imm = instr.i.rs1 & 0x1F;
    string_printf(s_comments, "%s = %s, %s = %u", rd, csr, csr, imm);
}
extern void rv_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrsi");
    csr_imm_instr_mnemonics(instr, s_mnemonics);

    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    uint32_t imm = instr.i.rs1 & 0x1F;
    string_printf(s_comments, "%s = %s, %s |= 0x%08x", rd, csr, csr, imm);
}
extern void rv_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrci");
    csr_imm_instr_mnemonics(instr, s_mnemonics);
    char* rd = rv_regnames[instr.i.rd];
    char* csr = rv_csrnames[RV_I_UNSIGNED_IMM(instr)];
    uint32_t imm = instr.i.rs1 & 0x1F;
    string_printf(s_comments, "%s = %s, %s &= 0x%08x", rd, csr, csr, ~imm);
}

// M extension
extern void rv_mul_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "mul");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "*");
    string_printf(s_comments, " (low 32 bits)");
}
extern void rv_mulh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "mulh");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "*");
    string_printf(s_comments, " (high 32 bits, both signed)");
}
extern void rv_mulhsu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "mulhsu");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "*");
    string_printf(s_comments, " (high 32 bits, signed and unsigned)");
}
extern void rv_mulhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "mulhu");
    r_instr_mnemonics(instr, s_mnemonics);
     r_instr_comment_binop(instr, s_comments, "*");
    string_printf(s_comments, " (high 32 bits, both unsigned)");
}
extern void rv_div_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "div");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "/");
    string_printf(s_comments, " (signed)");
}
extern void rv_divu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "divu");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "/");
    string_printf(s_comments, " (unsigned)");
}
extern void rv_rem_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "rem");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "%");
    string_printf(s_comments, " (signed)");
}
extern void rv_remu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "remu");
    r_instr_mnemonics(instr, s_mnemonics);
    r_instr_comment_binop(instr, s_comments, "%");
    string_printf(s_comments, " (unsigned)");
}

// A extension
extern void rv_lr_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "lr");
    string_printf(s_mnemonics, " %s, (%s)", rv_regnames[instr.r.rd], rv_regnames[instr.r.rs1]);
}
extern void rv_sc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "sc");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amoswap_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amoswap");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amoadd_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amoadd");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amoxor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amoxor");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amoand_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amoand");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amoor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amoor");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amomin_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amomin");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amomax_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amomax");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amominu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amominu");
    amo_instr_mnemonics(instr, s_mnemonics);
}
extern void rv_amomaxu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "amomaxu");
    amo_instr_mnemonics(instr, s_mnemonics);
}

#define default_print_function(csr_name) 													\
	static void print_##csr_name(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments){ 	\
		string_printf(mnemonics, "%s 0x%08x", #csr_name, cpu->csr.csr_name);				\
	}	

static void print_64_reg(uint64_t val, const char* name, string_t* s){
	string_printf(s, "%s 0x%016lx (%sh = 0x%08x, %s = 0x%08x)", name, val, name, (uint32_t)(val >> 32), name, (uint32_t)val);
}

static void print_cycle(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments){
	print_64_reg(cpu->csr.cycle, "cycle", mnemonics);
}

static void print_time(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	print_64_reg(cpu->csr.mtime, "time", mnemonics);
}

static void print_instret(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments){
	print_64_reg(cpu->csr.instret, "instret", mnemonics);
}

static void print_hpm(rv_cpu_t *cpu, int hpm, string_t* mnemonics, string_t* comments){
	ASSERT((hpm >= 3 && hpm < 32));
	string_t s;
	string_init(&s);
	string_printf(&s, "hpmcounter%i", hpm);
	print_64_reg(cpu->csr.hpmcounters[hpm - 3], s.str, mnemonics);
}

static void print_hpm_event(rv_cpu_t *cpu, int hpm, string_t* mnemonics, string_t* comments){
	ASSERT((hpm >= 3 && hpm < 32));
	string_t s;
	string_init(&s);
	string_printf(&s, "mhpmevent%i", hpm);

	string_printf(mnemonics, "%s 0x%08x", s.str, cpu->csr.hpmevents[hpm-3]);

	char* event_name;
	switch(cpu->csr.hpmevents[hpm-3]){
		case hpm_no_event:
			event_name = "no event";
			break;
		case hpm_u_cycles:
			event_name = "U mode cycles";
			break;
		case hpm_s_cycles:
			event_name = "S mode cycles";
			break;
		case hpm_m_cycles:
			event_name = "M mode cycles";
			break;
		case hpm_w_cycles:
			event_name = "Idle cycles";
			break;
		default:
			event_name = "Invalid event";
			break;
	}

	string_printf(comments, "%s", event_name);
}

#define bit_string(b) (b ? "1" : "0")

static void print_sstatus(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	uint32_t sstatus = cpu->csr.mstatus & rv_csr_sstatus_mask;

	bool sd = cpu->csr.mstatus & 0x80000000;
	bool mxr = rv_csr_sstatus_mxr(cpu);
	bool sum = rv_csr_sstatus_sum(cpu);
	int xs = (cpu->csr.mstatus & 0x18000) >> 15;
	int fs = (cpu->csr.mstatus & 0x6000) >> 13;
	int vs = (cpu->csr.mstatus & 0x600) >> 9;

	rv_priv_mode_t spp = rv_csr_sstatus_spp(cpu);
	char* spp_s = ((spp == rv_smode) ? "S" : "U");
	bool ube = rv_csr_sstatus_ube(cpu);
	bool spie = rv_csr_sstatus_spie(cpu);
	bool sie = rv_csr_sstatus_sie(cpu);

	string_printf(mnemonics, "%s 0x%08x","sstatus",sstatus);
	
	string_printf(comments, "SD %s, MXR %s, SUM %s, XS %i%i, FS %i%i, VS %i%i, SPP %s, UBE %s, SPIE %s, SIE %s",
		bit_string(sd),
		bit_string(mxr),
		bit_string(sum),
		xs >> 1, xs & 1,
		fs >> 1, fs & 1,
		vs >> 1, vs & 1,
		spp_s,
		bit_string(ube),
		bit_string(spie),
		bit_string(sie)
	);
}

static void print_mstatus(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	bool mbe = cpu->csr.mstatus & rv_csr_mstatush_mbe_mask;
	bool sbe = cpu->csr.mstatus & rv_csr_mstatush_sbe_mask;
	
	bool sd = cpu->csr.mstatus & 0x80000000;
	bool tsr = rv_csr_mstatus_tsr(cpu);
	bool tw = rv_csr_mstatus_tw(cpu);
	bool tvm = rv_csr_mstatus_tvm(cpu);
	bool mxr = rv_csr_sstatus_mxr(cpu);
	bool sum = rv_csr_sstatus_sum(cpu);
	bool mprv = rv_csr_mstatus_mprv(cpu);
	int xs = (cpu->csr.mstatus & 0x18000) >> 15;
	int fs = (cpu->csr.mstatus & 0x6000) >> 13;
	rv_priv_mode_t mpp = rv_csr_mstatus_mpp(cpu);
	char* mpp_s = (mpp == rv_mmode) ? "M" : ((mpp == rv_hmode) ? "H" : ((mpp == rv_smode) ? "S" : "U"));
	int vs = (cpu->csr.mstatus & 0x600) >> 9;
	rv_priv_mode_t spp = rv_csr_sstatus_spp(cpu);
	char* spp_s = ((spp == rv_smode) ? "S" : "U");
	bool mpie = rv_csr_mstatus_mpie(cpu);
	bool ube = rv_csr_sstatus_ube(cpu);
	bool spie = rv_csr_sstatus_spie(cpu);
	bool mie = rv_csr_mstatus_mie(cpu);
	bool sie = rv_csr_sstatus_sie(cpu);

	print_64_reg(cpu->csr.mstatus, "mstatus", mnemonics);

	string_printf(
		comments,
		"MBE %s, SBE %s, SD %s, TSR %s, TW %s, TVM %s, MXR %s, SUM %s, MPRV %s, XS %i%i, FS %i%i, MPP %s, VS %i%i, SPP %s, MPIE %s, UBE %s, SPIE %s, MIE %s, SIE %s",
		bit_string(mbe),
		bit_string(sbe),
		bit_string(sd),
		bit_string(tsr),
		bit_string(tw),
		bit_string(tvm),
		bit_string(mxr),
		bit_string(sum),
		bit_string(mprv),
		xs >> 1, xs & 1,
		fs >> 1, fs & 1,
		mpp_s,
		vs >> 1, vs & 1,
		spp_s,
		bit_string(mpie),
		bit_string(ube),
		bit_string(spie),
		bit_string(mie),
		bit_string(sie)
	);
}

static void print_misa(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	uint32_t misa = cpu->csr.misa;
	string_printf(mnemonics, "%s 0x%08x", "misa", misa);

	int mxl = 16 << (misa >> 30);
	
	string_printf(comments, "Machine XLEN: %i Extensions: ", mxl);
	if(misa & RV_A_EXTENSION_BITS) string_printf(comments, "A");
	if(misa & RV_C_EXTENSION_BITS) string_printf(comments, "C");
	if(misa & RV_D_EXTENSION_BITS) string_printf(comments, "D");
	if(misa & RV_E_EXTENSION_BITS) string_printf(comments, "E");
	if(misa & RV_F_EXTENSION_BITS) string_printf(comments, "F");
	if(misa & RV_H_EXTENSION_BITS) string_printf(comments, "H");
	if(misa & RV_I_EXTENSION_BITS) string_printf(comments, "I");
	if(misa & RV_M_EXTENSION_BITS) string_printf(comments, "M");
	if(misa & RV_Q_EXTENSION_BITS) string_printf(comments, "Q");
	if(misa & RV_S_IMPLEMENTED_BITS) string_printf(comments, "S");
	if(misa & RV_U_IMPLEMENTED_BITS) string_printf(comments, "U");
}

static void print_mtvec(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "mtvec", cpu->csr.mtvec);
	string_printf(comments,
		"Base: 0x%08x Mode: %s",
		cpu->csr.mtvec & ~0b11,
		(((cpu->csr.mtvec & 0b11) == 0 ) ? "Direct" : "Vectored")
	);
}

static void print_medeleg(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "medeleg", cpu->csr.medeleg);
	if(cpu->csr.medeleg == 0) return;
	string_printf(comments, "Delegated:");

	//TODO: remove trailing comma

	#define comment_if_ex_delegated(ex) if(cpu->csr.medeleg & RV_EXCEPTION_MASK(rv_exc_ ## ex)) string_printf(comments, " %s,", rv_excnames[rv_exc_ ## ex]);

	comment_if_ex_delegated(instruction_address_misaligned);
	comment_if_ex_delegated(instruction_access_fault);
	comment_if_ex_delegated(illegal_instruction);
	comment_if_ex_delegated(breakpoint);
	comment_if_ex_delegated(load_address_misaligned);
	comment_if_ex_delegated(load_access_fault);
	comment_if_ex_delegated(store_amo_address_misaligned);
	comment_if_ex_delegated(store_amo_access_fault);
	comment_if_ex_delegated(umode_environment_call);
	comment_if_ex_delegated(smode_environment_call);
	comment_if_ex_delegated(mmode_environment_call);
	comment_if_ex_delegated(instruction_page_fault);
	comment_if_ex_delegated(load_page_fault);
	comment_if_ex_delegated(store_amo_page_fault);
}

static void print_mideleg(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "mideleg", cpu->csr.mideleg);
	if(cpu->csr.mideleg == 0) return;
	string_printf(comments, "Delegated:");

	//TODO: remove trailing comma
	#define comment_if_i_delegated(i) if(cpu->csr.mideleg & RV_EXCEPTION_MASK(rv_exc_ ## i)) string_printf(comments, " %s,", rv_interruptnames[rv_exc_ ## i & ~RV_INTERRUPT_EXC_BITS]);

	comment_if_i_delegated(machine_external_interrupt);
	comment_if_i_delegated(supervisor_external_interrupt);
	comment_if_i_delegated(machine_timer_interrupt);
	comment_if_i_delegated(supervisor_timer_interrupt);
	comment_if_i_delegated(machine_software_interrupt);
	comment_if_i_delegated(supervisor_software_interrupt);
}

static void print_mie(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {

	bool meie = cpu->csr.mie & rv_csr_mei_mask;
	bool seie = cpu->csr.mie & rv_csr_sei_mask;
	bool mtie = cpu->csr.mie & rv_csr_mti_mask;
	bool stie = cpu->csr.mie & rv_csr_sti_mask;
	bool msie = cpu->csr.mie & rv_csr_msi_mask;
	bool ssie = cpu->csr.mie & rv_csr_ssi_mask;

	string_printf(mnemonics, "%s 0x%08x", "mie", cpu->csr.mie);
	string_printf(comments,
		"MEIE %s, SEIE %s, MTIE %s, STIE %s, MSIE %s, SSIE %s",
		bit_string(meie),
		bit_string(seie),
		bit_string(mtie),
		bit_string(stie),
		bit_string(msie),
		bit_string(ssie)
	);
}

static void print_mip(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {

	bool meip = cpu->csr.mip & rv_csr_mei_mask;
	bool seip = cpu->csr.mip & rv_csr_sei_mask;
	bool mtip = cpu->csr.mip & rv_csr_mti_mask;
	bool stip = cpu->csr.mip & rv_csr_sti_mask;
	bool msip = cpu->csr.mip & rv_csr_msi_mask;
	bool ssip = cpu->csr.mip & rv_csr_ssi_mask;

	string_printf(mnemonics, "%s 0x%08x", "mip", cpu->csr.mip);
	string_printf(comments,
		"MEIP %s, SEIP %s, MTIP %s, STIP %s, MSIP %s, SSIP %s (External SEIP %s)",
		bit_string(meip),
		bit_string(seip),
		bit_string(mtip),
		bit_string(stip),
		bit_string(msip),
		bit_string(ssip),
		bit_string(cpu->csr.external_SEIP)
	);
}

static void print_mcause(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "mcause", cpu->csr.mcause);
	bool is_interrupt = cpu->csr.mcause & RV_INTERRUPT_EXC_BITS;
	int cause_id = cpu->csr.mcause & ~RV_INTERRUPT_EXC_BITS;
	string_printf(comments, "%s", (is_interrupt ? rv_interruptnames[cause_id] : rv_excnames[cause_id]));
}

static void print_mseccfg(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "mseccfg", cpu->csr.mseccfg);
	bool sseed = cpu->csr.mseccfg & (1 << 9);
	bool useed = cpu->csr.mseccfg & (1 << 8);
	bool rlb = cpu->csr.mseccfg & (1 << 2);
	bool mmwp = cpu->csr.mseccfg & (1 << 1);
	bool mml = cpu->csr.mseccfg & (1 << 0);

	string_printf(comments,
		"SSEED %s, USEED %s, RLB %s, MMWP %s, MML %s",
		bit_string(sseed),
		bit_string(useed),
		bit_string(rlb),
		bit_string(mmwp),
		bit_string(mml)
	);
}

static void print_menvcfg(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	print_64_reg(cpu->csr.menvcfg, "menvcfg", mnemonics);
	bool stce = cpu->csr.menvcfg & (UINT64_C(1) << 63);
	bool pbmte = cpu->csr.menvcfg & (UINT64_C(1) << 62);
	bool cbze = cpu->csr.menvcfg & (UINT64_C(1) << 7);
	bool cbfe = cpu->csr.menvcfg & (UINT64_C(1) << 6);
	bool cbie = cpu->csr.menvcfg & (UINT64_C(1) << 5);
	bool fiom = cpu->csr.menvcfg & (UINT64_C(1) << 0);

	string_printf(comments,
		"STCE %s, PBMTE %s, CBZE %s, CBFE %s, CBIE %s, FIOM %s",
		bit_string(stce),
		bit_string(pbmte),
		bit_string(cbze),
		bit_string(cbfe),
		bit_string(cbie),
		bit_string(fiom)
	);
}

static void print_stvec(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments){
	string_printf(mnemonics, "%s 0x%08x", "stvec", cpu->csr.stvec);
	string_printf(comments, "Base: 0x%08x Mode: %s",
		cpu->csr.stvec & ~0b11,
		(((cpu->csr.stvec & 0b11) == 0) ? "Direct" : "Vectored")
	); 
}


static void print_sie(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {

	bool seie = cpu->csr.mie & rv_csr_sei_mask;
	bool stie = cpu->csr.mie & rv_csr_sti_mask;
	bool ssie = cpu->csr.mie & rv_csr_ssi_mask;

	string_printf(mnemonics, "%s 0x%08x", "sie", cpu->csr.mie & rv_csr_si_mask);
	string_printf(comments,
		"SEIE %s, STIE %s, SSIE %s",
		bit_string(seie),
		bit_string(stie),
		bit_string(ssie)
	);
}

static void print_sip(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {

	bool seip = cpu->csr.mip & rv_csr_sei_mask;
	bool stip = cpu->csr.mip & rv_csr_sti_mask;
	bool ssip = cpu->csr.mip & rv_csr_ssi_mask;

	string_printf(mnemonics, "%s 0x%08x", "sip", cpu->csr.mip & rv_csr_si_mask);
	string_printf(comments,
		"SEIP %s, STIP %s, SSIP %s (External SEIP %s)",
		bit_string(seip),
		bit_string(stip),
		bit_string(ssip),
		bit_string(cpu->csr.external_SEIP)
	);
}

static void print_scause(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "scause", cpu->csr.scause);
	bool is_interrupt = cpu->csr.scause & RV_INTERRUPT_EXC_BITS;
	int cause_id = cpu->csr.scause & ~RV_INTERRUPT_EXC_BITS;
	string_printf(comments, "%s", (is_interrupt ? rv_interruptnames[cause_id] : rv_excnames[cause_id]));
}

static void print_senvcfg(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "senvcfg", cpu->csr.senvcfg);
	bool cbze = cpu->csr.senvcfg & (UINT32_C(1) << 7);
	bool cbfe = cpu->csr.senvcfg & (UINT32_C(1) << 6);
	bool cbie = cpu->csr.senvcfg & (UINT32_C(1) << 5);
	bool fiom = cpu->csr.senvcfg & (UINT32_C(1) << 0);

	string_printf(comments,
		"CBZE %s, CBFE %s, CBIE %s, FIOM %s",
		bit_string(cbze),
		bit_string(cbfe),
		bit_string(cbie),
		bit_string(fiom)
	);
}

static void print_satp(rv_cpu_t *cpu, string_t* mnemonics, string_t* comments) {
	string_printf(mnemonics, "%s 0x%08x", "satp", cpu->csr.satp);
	char* mode = (rv_csr_satp_is_bare(cpu)) ? "Bare" : "Sv32";
	
	string_printf(comments, "Mode: %s", mode);
	if(!rv_csr_satp_is_bare(cpu)){
		int asid = rv_csr_satp_asid(cpu);
		uint32_t ppn =  rv_csr_satp_ppn(cpu);
		string_printf(comments,
			" ASID: %i PPN: 0x%06x (Physical address: 0x%09lx)",
			asid,
			ppn,
			(uint64_t)ppn << 12
		);
	}
}

default_print_function(mvendorid)
default_print_function(marchid)
default_print_function(mimpid)
default_print_function(mhartid)
default_print_function(mcounteren)
default_print_function(mcountinhibit)
default_print_function(mepc)
default_print_function(mscratch)
default_print_function(mtval)
default_print_function(mconfigptr)
default_print_function(mcontext)
default_print_function(scounteren)
default_print_function(sscratch)
default_print_function(sepc)
default_print_function(stval)
default_print_function(scontext)
default_print_function(scyclecmp)

void rv_csr_dump_common(rv_cpu_t *cpu, csr_num_t csr) {
	string_t s_mnemonics;
	string_t s_comments;

	string_init(&s_mnemonics);
	string_init(&s_comments);

	#define default_case(csr) 								\
		case csr_##csr:										\
			print_##csr(cpu, &s_mnemonics, &s_comments);	\
			break;

	switch(csr){
		case csr_cycleh:
		case csr_mcycle:
		case csr_mcycleh:
		default_case(cycle)

		case csr_timeh:
		default_case(time)
		
		case csr_instreth:
		case csr_minstret:
		case csr_minstreth:
		default_case(instret)

		case csr_hpmcounter3:
		case csr_hpmcounter4:
		case csr_hpmcounter5:
		case csr_hpmcounter6:
		case csr_hpmcounter7:
		case csr_hpmcounter8:
		case csr_hpmcounter9:
		case csr_hpmcounter10:
		case csr_hpmcounter11:
		case csr_hpmcounter12:
		case csr_hpmcounter13:
		case csr_hpmcounter14:
		case csr_hpmcounter15:
		case csr_hpmcounter16:
		case csr_hpmcounter17:
		case csr_hpmcounter18:
		case csr_hpmcounter19:
		case csr_hpmcounter20:
		case csr_hpmcounter21:
		case csr_hpmcounter22:
		case csr_hpmcounter23:
		case csr_hpmcounter24:
		case csr_hpmcounter25:
		case csr_hpmcounter26:
		case csr_hpmcounter27:
		case csr_hpmcounter28:
		case csr_hpmcounter29:
		case csr_hpmcounter30:
		case csr_hpmcounter31:
		case csr_hpmcounter3h:
		case csr_hpmcounter4h:
		case csr_hpmcounter5h:
		case csr_hpmcounter6h:
		case csr_hpmcounter7h:
		case csr_hpmcounter8h:
		case csr_hpmcounter9h:
		case csr_hpmcounter10h:
		case csr_hpmcounter11h:
		case csr_hpmcounter12h:
		case csr_hpmcounter13h:
		case csr_hpmcounter14h:
		case csr_hpmcounter15h:
		case csr_hpmcounter16h:
		case csr_hpmcounter17h:
		case csr_hpmcounter18h:
		case csr_hpmcounter19h:
		case csr_hpmcounter20h:
		case csr_hpmcounter21h:
		case csr_hpmcounter22h:
		case csr_hpmcounter23h:
		case csr_hpmcounter24h:
		case csr_hpmcounter25h:
		case csr_hpmcounter26h:
		case csr_hpmcounter27h:
		case csr_hpmcounter28h:
		case csr_hpmcounter29h:
		case csr_hpmcounter30h:
		case csr_hpmcounter31h:
		case csr_mhpmcounter3:
		case csr_mhpmcounter4:
		case csr_mhpmcounter5:
		case csr_mhpmcounter6:
		case csr_mhpmcounter7:
		case csr_mhpmcounter8:
		case csr_mhpmcounter9:
		case csr_mhpmcounter10:
		case csr_mhpmcounter11:
		case csr_mhpmcounter12:
		case csr_mhpmcounter13:
		case csr_mhpmcounter14:
		case csr_mhpmcounter15:
		case csr_mhpmcounter16:
		case csr_mhpmcounter17:
		case csr_mhpmcounter18:
		case csr_mhpmcounter19:
		case csr_mhpmcounter20:
		case csr_mhpmcounter21:
		case csr_mhpmcounter22:
		case csr_mhpmcounter23:
		case csr_mhpmcounter24:
		case csr_mhpmcounter25:
		case csr_mhpmcounter26:
		case csr_mhpmcounter27:
		case csr_mhpmcounter28:
		case csr_mhpmcounter29:
		case csr_mhpmcounter30:
		case csr_mhpmcounter31:
		case csr_mhpmcounter3h:
		case csr_mhpmcounter4h:
		case csr_mhpmcounter5h:
		case csr_mhpmcounter6h:
		case csr_mhpmcounter7h:
		case csr_mhpmcounter8h:
		case csr_mhpmcounter9h:
		case csr_mhpmcounter10h:
		case csr_mhpmcounter11h:
		case csr_mhpmcounter12h:
		case csr_mhpmcounter13h:
		case csr_mhpmcounter14h:
		case csr_mhpmcounter15h:
		case csr_mhpmcounter16h:
		case csr_mhpmcounter17h:
		case csr_mhpmcounter18h:
		case csr_mhpmcounter19h:
		case csr_mhpmcounter20h:
		case csr_mhpmcounter21h:
		case csr_mhpmcounter22h:
		case csr_mhpmcounter23h:
		case csr_mhpmcounter24h:
		case csr_mhpmcounter25h:
		case csr_mhpmcounter26h:
		case csr_mhpmcounter27h:
		case csr_mhpmcounter28h:
		case csr_mhpmcounter29h:
		case csr_mhpmcounter30h:
		case csr_mhpmcounter31h:
			print_hpm(cpu, csr & 0x1F, &s_mnemonics, &s_comments);
			break;
		case csr_mhpmevent3:
		case csr_mhpmevent4:
		case csr_mhpmevent5:
		case csr_mhpmevent6:
		case csr_mhpmevent7:
		case csr_mhpmevent8:
		case csr_mhpmevent9:
		case csr_mhpmevent10:
		case csr_mhpmevent11:
		case csr_mhpmevent12:
		case csr_mhpmevent13:
		case csr_mhpmevent14:
		case csr_mhpmevent15:
		case csr_mhpmevent16:
		case csr_mhpmevent17:
		case csr_mhpmevent18:
		case csr_mhpmevent19:
		case csr_mhpmevent20:
		case csr_mhpmevent21:
		case csr_mhpmevent22:
		case csr_mhpmevent23:
		case csr_mhpmevent24:
		case csr_mhpmevent25:
		case csr_mhpmevent26:
		case csr_mhpmevent27:
		case csr_mhpmevent28:
		case csr_mhpmevent29:
		case csr_mhpmevent30:
		case csr_mhpmevent31:
			print_hpm_event(cpu, csr & 0x1F, &s_mnemonics, &s_comments);
			break;
		
		default_case(misa)
		default_case(mvendorid)
		default_case(marchid)
		default_case(mimpid)
		default_case(mhartid)
		default_case(mtvec)
		default_case(medeleg)
		default_case(mideleg)
		default_case(mie)
		default_case(mip)
		default_case(mcounteren)
		default_case(mcountinhibit)
		default_case(mscratch)
		default_case(mepc)
		default_case(mcause)
		default_case(mtval)
		default_case(mconfigptr)
        default_case(mcontext)
		default_case(sstatus) 
		default_case(stvec)
		default_case(sie)
		default_case(sip)
		default_case(scounteren)
		default_case(sscratch)
		default_case(sepc)
		default_case(scause)
		default_case(stval)
		default_case(senvcfg)
		default_case(satp)
		default_case(scontext)
		default_case(scyclecmp)

		case csr_mstatush:
		default_case(mstatus)

		case csr_mseccfgh:
		default_case(mseccfg)

		case csr_menvcfgh:
		default_case(menvcfg)

		default:
			printf("Not implemented CSR number!\n");
			return;
		
	}

	printf("%s", s_mnemonics.str);

	if(icmt && s_comments.size > 0 && s_comments.str[0] != 0){
		printf(" [ %s ]", s_comments.str);
	}

	printf("\n");

	string_done(&s_mnemonics);
	string_done(&s_comments);

	#undef default_case
}

