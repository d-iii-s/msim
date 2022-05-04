#include "mnemonics.h"
#include "debug.h"
#include "instr.h"
#include "instructions/computations.h"
#include "instructions/control_transfer.h"
#include "instructions/system.h"
#include "instructions/mem_ops.h"

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

    if(instr_func == break_instr)
        return rv_ebreak_mnemonics;
    
    if(instr_func == halt_instr)
        return rv_ehalt_mnemonics;

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

    // TODO: add rest of instructions

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
    string_printf(s_mnemonics, " %s, %#08x", rv_regnames[instr.u.rd], instr.u.imm);
}

static void amo_instr_mnemonics(rv_instr_t instr, string_t *s_mnemonics) {
    string_printf(s_mnemonics, " %s, %s, (%s)", rv_regnames[instr.r.rd], rv_regnames[instr.r.rs2], rv_regnames[instr.r.rs1]);
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

// CSR
extern void rv_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrw");
}
extern void rv_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrs");
}
extern void rv_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrc");
}
extern void rv_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrwi");
}
extern void rv_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrsi");
}
extern void rv_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments){
    string_printf(s_mnemonics, "csrrci");
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