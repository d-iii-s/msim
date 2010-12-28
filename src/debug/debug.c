/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Useful debugging features
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "debug.h"
#include "../cpu/instr.h"
#include "../cpu/r4000.h"
#include "../device/dcpu.h"
#include "../device/mem.h"
#include "../assert.h"
#include "../env.h"
#include "../main.h"
#include "../utils.h"

#define CP0_PM_ITEMS  7
#define REG_BUF       1024

/** Instruction decoding tables and mnemonics
 *
 */
typedef void (*mnemonics_fnc_t)(string_t *, string_t *, cpu_t *, instr_t);

static void mnemonics__reserved(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "(reserved)");
}

static void mnemonics_beq(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "beq");
}

static void mnemonics_bne(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bne");
}

static void mnemonics_blez(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "blez");
}

static void mnemonics_bgtz(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgtz");
}

static void mnemonics_add(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "add");
}

static void mnemonics_addi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "addi");
}

static void mnemonics_addiu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "addiu");
}

static void mnemonics_addu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "addu");
}

static void mnemonics_and(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "and");
}

static void mnemonics_andi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "andi");
}

static void mnemonics_bc0f(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc0f");
}

static void mnemonics_bc0fl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc0fl");
}

static void mnemonics_bc0t(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc0t");
}

static void mnemonics_bc0tl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc0tl");
}

static void mnemonics_bc1f(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc1f");
}

static void mnemonics_bc1fl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc1fl");
}

static void mnemonics_bc1t(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc1t");
}

static void mnemonics_bc1tl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc1tl");
}

static void mnemonics_bc2f(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc2f");
}

static void mnemonics_bc2fl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc2fl");
}

static void mnemonics_bc2t(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc2t");
}

static void mnemonics_bc2tl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bc2tl");
}

static void mnemonics_beql(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "beql");
}

static void mnemonics_bgez(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgez");
}

static void mnemonics_bgezal(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgezal");
}

static void mnemonics_bgezall(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgezall");
}

static void mnemonics_bgezl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgezl");
}

static void mnemonics_bgtzl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bgtzl");
}

static void mnemonics_blezl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "blezl");
}

static void mnemonics_bltz(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bltz");
}

static void mnemonics_bltzal(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bltzal");
}

static void mnemonics_bltzall(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bltzall");
}

static void mnemonics_bltzl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bltzl");
}

static void mnemonics_bnel(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "bnel");
}

static void mnemonics_break(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "break");
}

static void mnemonics_cache(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "cache");
}

static void mnemonics_cfc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "cfc1");
}

static void mnemonics_cfc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "cfc2");
}

static void mnemonics_ctc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ctc1");
}

static void mnemonics_ctc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ctc2");
}

static void mnemonics_dadd(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dadd");
}

static void mnemonics_daddi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "daddi");
}

static void mnemonics_daddiu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "daddiu");
}

static void mnemonics_daddu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "daddu");
}

static void mnemonics_ddiv(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ddiv");
}

static void mnemonics_ddivu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ddivu");
}

static void mnemonics_div(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "div");
}

static void mnemonics_divu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "divu");
}

static void mnemonics_dmfc0(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmfc0");
}

static void mnemonics_dmfc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmfc1");
}

static void mnemonics_dmtc0(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmtc0");
}

static void mnemonics_dmtc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmtc1");
}

static void mnemonics_dmult(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmult");
}

static void mnemonics_dmultu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dmultu");
}

static void mnemonics_dsll(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsll");
}

static void mnemonics_dsll32(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsll32");
}

static void mnemonics_dsllv(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsllv");
}

static void mnemonics_dsra(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsra");
}

static void mnemonics_dsra32(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsra32");
}

static void mnemonics_dsrav(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsrav");
}

static void mnemonics_dsrl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsrl");
}

static void mnemonics_dsrl32(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsrl32");
}

static void mnemonics_dsrlv(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsrlv");
}

static void mnemonics_dsub(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsub");
}

static void mnemonics_dsubu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "dsubu");
}

static void mnemonics_eret(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "eret");
}

static void mnemonics_j(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "j");
}

static void mnemonics_jal(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "jal");
}

static void mnemonics_jalr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "jalr");
}

static void mnemonics_jr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "jr");
}

static void mnemonics_lb(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lb");
}

static void mnemonics_lbu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lbu");
}

static void mnemonics_ld(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ld");
}

static void mnemonics_ldc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ldc1");
}

static void mnemonics_ldc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ldc2");
}

static void mnemonics_ldl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ldl");
}

static void mnemonics_ldr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ldr");
}

static void mnemonics_lh(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lh");
}

static void mnemonics_lhu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lhu");
}

static void mnemonics_ll(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ll");
}

static void mnemonics_lld(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lld");
}

static void mnemonics_lui(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lui");
}

static void mnemonics_lw(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lw");
}

static void mnemonics_lwc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lwc1");
}

static void mnemonics_lwc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lwc2");
}

static void mnemonics_lwl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lwl");
}

static void mnemonics_lwr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lwr");
}

static void mnemonics_lwu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "lwu");
}

static void mnemonics_mfc0(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mfc0");
}

static void mnemonics_mfc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mfc1");
}

static void mnemonics_mfc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mfc2");
}

static void mnemonics_mfhi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mfhi");
}

static void mnemonics_mflo(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mflo");
}

static void mnemonics_mtc0(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mtc0");
}

static void mnemonics_mtc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mtc1");
}

static void mnemonics_mthi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mthi");
}

static void mnemonics_mtlo(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mtlo");
}

static void mnemonics_mult(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "mult");
}

static void mnemonics_multu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "multu");
}

static void mnemonics_nor(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "nor");
}

static void mnemonics_or(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "or");
}

static void mnemonics_ori(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "ori");
}

static void mnemonics_sb(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sb");
}

static void mnemonics_sc(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sc");
}

static void mnemonics_scd(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "scd");
}

static void mnemonics_sd(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sd");
}

static void mnemonics_sdc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sdc1");
}

static void mnemonics_sdc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sdc2");
}

static void mnemonics_sdl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sdl");
}

static void mnemonics_sdr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sdr");
}

static void mnemonics_sh(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sh");
}

static void mnemonics_sll(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sll");
}

static void mnemonics_sllv(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sllv");
}

static void mnemonics_slt(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "slt");
}

static void mnemonics_slti(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "slti");
}

static void mnemonics_sltiu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sltiu");
}

static void mnemonics_sltu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sltu");
}

static void mnemonics_sra(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sra");
}

static void mnemonics_srav(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "srav");
}

static void mnemonics_srl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "srl");
}

static void mnemonics_srlv(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "srlv");
}

static void mnemonics_sub(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sub");
}

static void mnemonics_subu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "subu");
}

static void mnemonics_sw(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sw");
}

static void mnemonics_swc1(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "swc1");
}

static void mnemonics_swc2(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "swc2");
}

static void mnemonics_swl(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "swl");
}

static void mnemonics_swr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "swr");
}

static void mnemonics_sync(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "sync");
}

static void mnemonics_syscall(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "syscall");
}

static void mnemonics_teq(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "teq");
}

static void mnemonics_teqi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "teqi");
}

static void mnemonics_tge(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tge");
}

static void mnemonics_tgei(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tgei");
}

static void mnemonics_tgeiu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tgeiu");
}

static void mnemonics_tgeu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tgeu");
}

static void mnemonics_tlbp(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlbp");
}

static void mnemonics_tlbr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlbr");
}

static void mnemonics_tlbwi(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlbwi");
}

static void mnemonics_tlbwr(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlbwr");
}

static void mnemonics_tlt(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlt");
}

static void mnemonics_tlti(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tlti");
}

static void mnemonics_tltiu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tltiu");
}

static void mnemonics_tltu(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tltu");
}

static void mnemonics_tne(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tne");
}

static void mnemonics_tnei(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "tnei");
}

static void mnemonics_xor(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "xor");
}

static void mnemonics_xori(string_t *mnemonics, string_t *comments,
    cpu_t *cpu, instr_t instr)
{
	string_printf(mnemonics, "xori");
}



static mnemonics_fnc_t opcode_map[64] = {
	/* 0 */
	mnemonics__reserved,  /* opcSPECIAL */
	mnemonics__reserved,  /* opcREGIMM */
	mnemonics_j,
	mnemonics_jal,
	mnemonics_beq,
	mnemonics_bne,
	mnemonics_blez,
	mnemonics_bgtz,
	
	/* 8 */
	mnemonics_addi,
	mnemonics_addiu,
	mnemonics_slti,
	mnemonics_sltiu,
	mnemonics_andi,
	mnemonics_ori,
	mnemonics_xori,
	mnemonics_lui,
	
	/* 16 */
	mnemonics__reserved,  /* opcCOP0 */
	mnemonics__reserved,  /* opcCOP1 */
	mnemonics__reserved,  /* opcCOP2 */
	mnemonics__reserved,  /* unused */
	mnemonics_beql,
	mnemonics_bnel,
	mnemonics_blezl,
	mnemonics_bgtzl,
	
	/* 24 */
	mnemonics_daddi,
	mnemonics_daddiu,
	mnemonics_ldl,
	mnemonics_ldr,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	/* 32 */
	mnemonics_lb,
	mnemonics_lh,
	mnemonics_lwl,
	mnemonics_lw,
	mnemonics_lbu,
	mnemonics_lhu,
	mnemonics_lwr,
	mnemonics_lwu,
	
	/* 40 */
	mnemonics_sb,
	mnemonics_sh,
	mnemonics_swl,
	mnemonics_sw,
	mnemonics_sdl,
	mnemonics_sdr,
	mnemonics_swr,
	mnemonics_cache,
	
	/* 48 */
	mnemonics_ll,
	mnemonics_lwc1,
	mnemonics_lwc2,
	mnemonics__reserved,  /* unused */
	mnemonics_lld,
	mnemonics_ldc1,
	mnemonics_ldc2,
	mnemonics_ld,
	
	mnemonics_sc,
	mnemonics_swc1,
	mnemonics_swc2,
	mnemonics__reserved,  /* unused */
	mnemonics_scd,
	mnemonics_sdc1,
	mnemonics_sdc2,
	mnemonics_sd
};

static mnemonics_fnc_t func_map[64] = {
	mnemonics_sll,
	mnemonics__reserved,  /* unused */
	mnemonics_srl,
	mnemonics_sra,
	mnemonics_sllv,
	mnemonics__reserved,  /* unused */
	mnemonics_srlv,
	mnemonics_srav,
	
	mnemonics_jr,
	mnemonics_jalr,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_syscall,
	mnemonics_break,
	mnemonics__reserved,  /* unused */
	mnemonics_sync,
	
	mnemonics_mfhi,
	mnemonics_mthi,
	mnemonics_mflo,
	mnemonics_mtlo,
	mnemonics_dsllv,
	mnemonics__reserved,  /* unused */
	mnemonics_dsrlv,
	mnemonics_dsrav,
	
	mnemonics_mult,
	mnemonics_multu,
	mnemonics_div,
	mnemonics_divu,
	mnemonics_dmult,
	mnemonics_dmultu,
	mnemonics_ddiv,
	mnemonics_ddivu,
	
	mnemonics_add,
	mnemonics_addu,
	mnemonics_sub,
	mnemonics_subu,
	mnemonics_and,
	mnemonics_or,
	mnemonics_xor,
	mnemonics_nor,
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_slt,
	mnemonics_sltu,
	mnemonics_dadd,
	mnemonics_daddu,
	mnemonics_dsub,
	mnemonics_dsubu,
	
	mnemonics_tge,
	mnemonics_tgeu,
	mnemonics_tlt,
	mnemonics_tltu,
	mnemonics_teq,
	mnemonics__reserved,  /* unused */
	mnemonics_tne,
	mnemonics__reserved,  /* unused */
	
	mnemonics_dsll,
	mnemonics__reserved,  /* unused */
	mnemonics_dsrl,
	mnemonics_dsra,
	mnemonics_dsll32,
	mnemonics__reserved,  /* unused */
	mnemonics_dsrl32,
	mnemonics_dsra32
};

static mnemonics_fnc_t rt_map[32] = {
	mnemonics_bltz,
	mnemonics_bgez,
	mnemonics_bltzl,
	mnemonics_bgezl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics_tgei,
	mnemonics_tgeiu,
	mnemonics_tlti,
	mnemonics_tltiu,
	mnemonics_teqi,
	mnemonics__reserved,  /* unused */
	mnemonics_tnei,
	mnemonics__reserved,  /* unused */
	
	mnemonics_bltzal,
	mnemonics_bgezal,
	mnemonics_bltzall,
	mnemonics_bgezall,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop0_rs_map[32] = {
	mnemonics_mfc0,
	mnemonics_dmfc0,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_mtc0,
	mnemonics_dmtc0,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop0rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop0rsCO */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop1_rs_map[32] = {
	mnemonics_mfc1,
	mnemonics_dmfc1,
	mnemonics_cfc1,
	mnemonics__reserved,  /* unused */
	mnemonics_mtc1,
	mnemonics_dmtc1,
	mnemonics_ctc1,
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop1rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop2_rs_map[32] = {
	mnemonics_mfc2,
	mnemonics__reserved,  /* unused */
	mnemonics_cfc2,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_ctc2,
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop2rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop0_rt_map[32] = {
	mnemonics_bc0f,
	mnemonics_bc0t,
	mnemonics_bc0fl,
	mnemonics_bc0tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop1_rt_map[32] = {
	mnemonics_bc1f,
	mnemonics_bc1t,
	mnemonics_bc1fl,
	mnemonics_bc1tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop2_rt_map[32] = {
	mnemonics_bc2f,
	mnemonics_bc2t,
	mnemonics_bc2fl,
	mnemonics_bc2tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t cop0_func_map[64] = {
	mnemonics__reserved,  /* unused */
	mnemonics_tlbr,
	mnemonics_tlbwi,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_tlbwr,
	mnemonics__reserved,  /* unused */
	
	mnemonics_tlbp,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics_eret,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static struct {
	uint32_t no;
	char *s;
} pagemask_name[CP0_PM_ITEMS + 1] = {
	{ 0x0U,   "4k" },
	{ 0x3U,   "16k" },
	{ 0xfU,   "64k" },
	{ 0x3fU,  "256k" },
	{ 0xffU,  "1M" },
	{ 0x3ffU, "4M" },
	{ 0xfffU, "16M" },
	{ -1,     "err" }
};

static char *cp0_dump_str[] = {
	"  00 Index\t%08X  index: %02X res: %x p: %01x \n",
	"  01 Random\t%08X  random: %02X, res: %07X\n",
	"  02 EntryLo0\t%08X  g: %x v: %x d: %x c: %x pfn: %06X res: %x\n",
	"  03 EntryLo1\t%08X  g: %x v: %x d: %x c: %x pfn: %06X res: %x\n",
	"  04 Context\t%08X  res: %x badvpn2: %05X ptebase: %03X\n",
	"  05 PageMask\t%08X  res1: %04x mask: %03X (%s) res2: %02X\n",
	"  06 Wired\t%08X  wired: %x res: %07X\n",
	"  07 Reserved\n",
	"  08 BadVAddr\t%08X\n",
	"  09 Count\t%08X\n",
	"  0a EntryHi\t%08X  asid: %02X res: %x vpn2: %05X\n",
	"  0b Compare\t%08X\n",
	"  0c Status\t%08X  ie: %x exl: %x erl: %x ksu: %x "
		"ux: %x sx: %x kx: %x\n\t\t\t  im: %02X de: %x "
		"ce: %x ch: %x res1: %x sr: %x ts: %x\n\t\t\t  "
		"bev: %x res2: %x re: %x fr: %x rp: %x cu: %x\n",
	"  0d Cause\t%08X  res1: %x exccode: %02X res2: %x "
		"ip: %02X res3: %02X\n\t\t\t  ce: %d res4: %d bd: %d\n",
	"  0e EPC\t%08X\n",
	"  0f PRId\t%08X  rev: %02X imp: %02X res: %04X\n",
	"  10 Config\t%08X  k0: %x cu: %x db: %x b: %x dc: %x "
		"ic: %x res: %x eb: %x\n\t\t\t  em: %x be: %x sm: %x sc: %x "
		"ew: %x sw: %x ss: %x sb: %x\n\t\t\t  ep: %x ec: %x cm: %x\n",
	"  11 LLAddr\t%08X\n",
	"  12 WatchLo\t%08X  w: %x r: %x res: %x paddr0: %08X\n",
	"  13 WatchHi\t%08X  res: %08X paddr1: %x\n",
	"  14 XContext\n",
	"  15 Reserved\n",
	"  16 Reserved\n",
	"  17 Reserved\n",
	"  18 Reserved\n",
	"  19 Reserved\n",
	"  1a Reserved\n",
	"  1b Reserved\n",
	"  1c Reserved\n",
	"  1d Reserved\n",
	"  1e ErrorEPC\t%08x  errorepc: %08x\n",
	"  1f Reserved\n"
};

void reg_dump(cpu_t *cpu)
{
	printf("processor %u\n", cpu->procno);
	
	unsigned int i;
	for (i = 0; i < 30; i += 5) {
		printf(" %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "\n",
		    regname[i],     cpu->regs[i].val,
		    regname[i + 1], cpu->regs[i + 1].val,
		    regname[i + 2], cpu->regs[i + 2].val,
		    regname[i + 3], cpu->regs[i + 3].val,
		    regname[i + 4], cpu->regs[i + 4].val);
	}
	
	printf(" %3s %16" PRIx64 "  %3s %16" PRIx64 "   pc %16" PRIx64 "   lo %16" PRIx64 "   hi %16" PRIx64 "\n",
	    regname[i],     cpu->regs[i].val,
	    regname[i + 1], cpu->regs[i + 1].val,
	    cpu->pc.ptr, cpu->loreg.val, cpu->hireg.val);
}

static const char *get_pagemask_name(unsigned int pm)
{
	unsigned int i;
	for (i = 0; i < CP0_PM_ITEMS; i++)
		if (pm == pagemask_name[i].no)
			return pagemask_name[i].s;
	
	/* Error */
	return pagemask_name[CP0_PM_ITEMS].s;
}

void tlb_dump(cpu_t *cpu)
{
	printf(" [             general             ][    subp 0     ][     subp 1    ]\n"
	    "  no    vpn      mask        g asid  v d   pfn     c  v d   pfn     c\n");
	
	unsigned int i;
	for (i = 0; i < 48; i++) {
		tlb_entry_t *e = &(cpu->tlb[i]);
		
		printf("  %02x  %08" PRIx32 " %08" PRIx32 ":%-4s %u  %02x   %u %u %09" PRIx64 " %1x  %u %u %09" PRIx64 " %1x\n",
		    i, e->vpn2, e->mask,
		    get_pagemask_name((~e->mask) >> cp0_pagemask_mask_shift),
		    e->global, e->asid, e->pg[0].valid, e->pg[0].dirty,
		    e->pg[0].pfn, e->pg[0].cohh, e->pg[1].valid,
		    e->pg[1].dirty, e->pg[1].pfn, e->pg[1].cohh);
	}
}

static void cp0_dump_reg(cpu_t *cpu, unsigned int reg)
{
	const char *s = cp0_dump_str[reg];
	
	switch (reg) {
	case cp0_Index:
		printf(s,
		    cp0_index(cpu),
		    cp0_index_index(cpu), cp0_index_res(cpu), cp0_index_p(cpu));
		break;
	case cp0_Random:
		printf(s,
		    cp0_random(cpu), cp0_random_random(cpu), cp0_random_res(cpu));
		break;
	case cp0_EntryLo0:
		printf(s,
		    cp0_entrylo0(cpu),
		    cp0_entrylo0_g(cpu), cp0_entrylo0_v(cpu),
		    cp0_entrylo0_d(cpu), cp0_entrylo0_c(cpu),
		    cp0_entrylo0_pfn(cpu), cp0_entrylo0_res1(cpu));
		break;
	case cp0_EntryLo1:
		printf(s,
		    cp0_entrylo1(cpu),
		    cp0_entrylo1_g(cpu), cp0_entrylo1_v(cpu),
		    cp0_entrylo1_d(cpu), cp0_entrylo1_c(cpu),
		    cp0_entrylo1_pfn(cpu), cp0_entrylo1_res1(cpu));
		break;
	case cp0_Context:
		printf(s,
		    cp0_context(cpu),
		    cp0_context_res1(cpu),
		    cp0_context_badvpn2(cpu),
		    cp0_context_ptebase(cpu));
		break;
	case cp0_PageMask:
		printf(s,
		    cp0_pagemask(cpu),
		    cp0_pagemask_res1(cpu),
		    cp0_pagemask_mask(cpu),
		    get_pagemask_name(cp0_pagemask_mask(cpu)),
		    cp0_pagemask_res2(cpu));
		break;
	case cp0_Wired:
		printf(s,
		    cp0_wired(cpu), cp0_wired_w(cpu), cp0_wired_res1(cpu));
		break;
	case cp0_BadVAddr:
		printf(s, cp0_badvaddr(cpu));
		break;
	case cp0_Count:
		printf(s, cp0_count(cpu));
		break;
	case cp0_EntryHi:
		printf(s,
		    cp0_entryhi(cpu), cp0_entryhi_asid(cpu),
		    cp0_entryhi_res1(cpu), cp0_entryhi_vpn2(cpu));
		break;
	case cp0_Compare:
		printf(s, cp0_compare(cpu));
		break;
	case cp0_Status:
		printf(s,
		    cp0_status(cpu),
		    cp0_status_ie(cpu), cp0_status_exl(cpu), cp0_status_erl(cpu),
		    cp0_status_ksu(cpu), cp0_status_ux(cpu), cp0_status_sx(cpu),
		    cp0_status_kx(cpu), cp0_status_im(cpu), cp0_status_de(cpu),
		    cp0_status_ce(cpu), cp0_status_ch(cpu), cp0_status_res1(cpu),
		    cp0_status_sr(cpu), cp0_status_ts(cpu), cp0_status_bev(cpu),
		    cp0_status_res2(cpu), cp0_status_re(cpu), cp0_status_fr(cpu),
		    cp0_status_rp(cpu), cp0_status_cu(cpu));
		break;
	case cp0_Cause:
		printf(s,
		    cp0_cause(cpu), cp0_cause_res1(cpu), cp0_cause_exccode(cpu),
		    cp0_cause_res2(cpu), cp0_cause_ip(cpu), cp0_cause_res3(cpu),
		    cp0_cause_ce(cpu), cp0_cause_res4(cpu), cp0_cause_bd(cpu));
		break;
	case cp0_EPC:
		printf(s, cp0_epc(cpu));
		break;
	case cp0_PRId:
		printf(s,
		    cp0_prid(cpu), cp0_prid_rev(cpu),
		    cp0_prid_imp(cpu), cp0_prid_res(cpu));
		break;
	case cp0_Config:
		printf(s,
		    cp0_config(cpu), cp0_config_k0(cpu), cp0_config_cu(cpu),
		    cp0_config_db(cpu), cp0_config_b(cpu), cp0_config_dc(cpu),
		    cp0_config_ic(cpu), cp0_config_res(cpu), cp0_config_eb(cpu),
		    cp0_config_em(cpu), cp0_config_be(cpu), cp0_config_sm(cpu),
		    cp0_config_sc(cpu), cp0_config_ew(cpu), cp0_config_sw(cpu),
		    cp0_config_ss(cpu), cp0_config_sb(cpu), cp0_config_ep(cpu),
		    cp0_config_ec(cpu), cp0_config_cm(cpu));
		break;
	case cp0_LLAddr:
		printf(s, cp0_lladdr(cpu));
		break;
	case cp0_WatchLo:
		printf(s,
		    cp0_watchlo(cpu), cp0_watchlo_w(cpu), cp0_watchlo_r(cpu),
		    cp0_watchlo_res(cpu), cp0_watchlo_paddr0(cpu));
		break;
	case cp0_WatchHi:
		printf(s,
		    cp0_watchhi(cpu), cp0_watchhi_paddr1(cpu), cp0_watchhi_res(cpu));
		break;
	case cp0_ErrorEPC:
		printf(s, cp0_errorepc(cpu), cp0_errorepc(cpu));
		break;
	default:
		printf(s);
		break;
	}
}

void cp0_dump_all(cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	printf("  no name       hex dump  readable dump\n");
	cp0_dump_reg(cpu, 0);
	cp0_dump_reg(cpu, 1);
	cp0_dump_reg(cpu, 2);
	cp0_dump_reg(cpu, 3);
	cp0_dump_reg(cpu, 4);
	cp0_dump_reg(cpu, 5);
	cp0_dump_reg(cpu, 6);
	cp0_dump_reg(cpu, 8);
	cp0_dump_reg(cpu, 9);
	cp0_dump_reg(cpu, 10);
	cp0_dump_reg(cpu, 11);
	cp0_dump_reg(cpu, 12);
	cp0_dump_reg(cpu, 13);
	cp0_dump_reg(cpu, 14);
	cp0_dump_reg(cpu, 15);
	cp0_dump_reg(cpu, 16);
	cp0_dump_reg(cpu, 17);
	cp0_dump_reg(cpu, 18);
	cp0_dump_reg(cpu, 19);
	cp0_dump_reg(cpu, 20);
	cp0_dump_reg(cpu, 30);
}

void cp0_dump(cpu_t *cpu, unsigned int reg)
{
	ASSERT(cpu != NULL);
	
	printf("  no name       hex dump  readable dump\n");
	cp0_dump_reg(cpu, reg);
}

/** Decode MIPS R4000 instruction mnemonics
 *
 * @return Instruction mnemonics function.
 *
 */
static mnemonics_fnc_t decode(instr_t instr)
{
	mnemonics_fnc_t fnc;
	
	/*
	 * Basic opcode decoding based
	 * on the opcode field.
	 */
	switch (instr.r.opcode) {
	case opcSPECIAL:
		/*
		 * SPECIAL opcode decoding based
		 * on the func field.
		 */
		fnc = func_map[instr.r.func];
		break;
	case opcREGIMM:
		/*
		 * REGIMM opcode decoding based
		 * on the rt field.
		 */
		fnc = rt_map[instr.r.rt];
		break;
	case opcCOP0:
		/*
		 * COP0 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop0rsBC:
			/*
			 * COP0/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop0_rt_map[instr.r.rt];
			break;
		case cop0rsCO:
			/*
			 * COP0/CO opcode decoding
			 * based on the 8-bit func field.
			 */
			fnc = cop0_func_map[instr.cop.func];
			break;
		default:
			fnc = cop0_rs_map[instr.r.rs];
		}
		break;
	case opcCOP1:
		/*
		 * COP1 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop1rsBC:
			/*
			 * COP1/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop1_rt_map[instr.r.rt];
			break;
		default:
			fnc = cop1_rs_map[instr.r.rs];
		}
		break;
	case opcCOP2:
		/*
		 * COP2 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop2rsBC:
			/*
			 * COP2/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop2_rt_map[instr.r.rt];
			break;
		default:
			fnc = cop2_rs_map[instr.r.rs];
		}
		break;
	default:
		fnc = opcode_map[instr.r.opcode];
	}
	
	return fnc;
}

static void idump_common(cpu_t *cpu, string_t *s_opc, string_t *s_mnemonics,
    string_t *s_comments, instr_t instr)
{
	string_printf(s_opc, "%08" PRIx32, instr.val);
	
	mnemonics_fnc_t fnc = decode(instr);
	fnc(s_mnemonics, s_comments, cpu, instr);
}

/** Dump instruction mnemonics
 *
 * @param cpu     If not NULL, then the dump is processor-dependent
 *                (with processor number).
 * @param addr    Virtual address of the instruction.
 * @param instr   Instruction to dump.
 * @param modregs If true, then modified registers are also dumped.
 *
 */
void idump(cpu_t *cpu, ptr64_t addr, instr_t instr, bool modregs)
{
	string_t s_cpu;
	string_t s_addr;
	string_t s_opc;
	string_t s_mnemonics;
	string_t s_comments;
	
	string_init(&s_cpu);
	string_init(&s_addr);
	string_init(&s_opc);
	string_init(&s_mnemonics);
	string_init(&s_comments);
	
	if (cpu != NULL)
		string_printf(&s_cpu, "cpu%u", cpu->procno);
	
	string_printf(&s_addr, "%#018" PRIx64, addr.ptr);
	idump_common(cpu, &s_opc, &s_mnemonics, &s_comments, instr);
	
	if (cpu != NULL)
		printf("%-5s ", s_cpu.str);
	
	if (iaddr)
		printf("%-18s ", s_addr.str);
	
	if (iopc)
		printf("%-8s ", s_opc.str);
	
	printf("%s\n", s_mnemonics.str);
	
	string_done(&s_cpu);
	string_done(&s_addr);
	string_done(&s_opc);
	string_done(&s_mnemonics);
	string_done(&s_comments);
}

/** Dump instruction mnemonics
 *
 * @param addr  Physical address of the instruction.
 * @param instr Instruction to dump.
 *
 */
void idump_phys(ptr36_t addr, instr_t instr)
{
	string_t s_addr;
	string_t s_iopc;
	string_t s_mnemonics;
	string_t s_comments;
	
	string_init(&s_addr);
	string_init(&s_iopc);
	string_init(&s_mnemonics);
	string_init(&s_comments);
	
	if (iaddr)
		string_printf(&s_addr, "%#011" PRIx64, addr);
	
	idump_common(NULL, &s_iopc, &s_mnemonics, &s_comments, instr);
	
	string_done(&s_addr);
	string_done(&s_iopc);
	string_done(&s_mnemonics);
	string_done(&s_comments);
}

/** Write info about changed registers
 *
 * Each modified register is included to the output.
 *
 */
char *modified_regs_dump(cpu_t *cpu)
{
	unsigned int i;
	char *s1;
	char *s2;
	char *s3;
	char sc1[REG_BUF];
	char sc2[REG_BUF];
	
	char *sx = safe_malloc(REG_BUF);
	size_t size = REG_BUF;
	
	sc1[0] = 0;
	sc2[0] = 0;
	s1 = sc1;
	s2 = sc2;
	
	/* Test for general registers */
	for (i = 0; i < 32; i++)
		if (cpu->regs[i].val != cpu->old_regs[i].val) {
			snprintf(s1, size, "%s, %s: %#" PRIx64 "->%#" PRIx64,
			    s2, regname[i], cpu->old_regs[i].val, cpu->regs[i].val);
			
			s3 = s1;
			s1 = s2;
			s2 = s3;
			cpu->old_regs[i] = cpu->regs[i];
		}
		
	/* Test for cp0 */
	for (i = 0; i < 32; i++)
		if ((cpu->cp0[i].val != cpu->old_cp0[i].val) && (i != cp0_Random) && (i != cp0_Count)) {
			if (cp0name == cp0_name[2])
				snprintf(s1, size, "%s, cp0_%s: %#" PRIx64 "->%#" PRIx64,
				    s2, cp0name[i], cpu->old_cp0[i].val, cpu->cp0[i].val);
			else
				snprintf(s1, size, "%s, cp0[%u]: %#" PRIx64 "->%#" PRIx64,
				    s2, i, cpu->old_cp0[i].val, cpu->cp0[i].val);
			
			s3 = s1;
			s1 = s2;
			s2 = s3;
			cpu->old_cp0[i] = cpu->cp0[i];
		}
	
	/* Test for loreg */
	if (cpu->loreg.val != cpu->old_loreg.val) {
		snprintf(s1, size, "%s, loreg: %#" PRIx64 "->%#" PRIx64,
		    s2, cpu->old_loreg.val, cpu->loreg.val);
		
		s3 = s1;
		s1 = s2;
		s2 = s3;
		cpu->old_loreg = cpu->loreg;
	}
	
	/* Test for hireg */
	if (cpu->hireg.val != cpu->old_hireg.val) {
		snprintf(s1, size, "%s, hireg: %#" PRIx64 "->%#" PRIx64,
		    s2, cpu->old_hireg.val, cpu->hireg.val);
		
		s3 = s1;
		s1 = s2;
		s2 = s3;
		cpu->old_hireg = cpu->hireg;
	}
	
	if (*s2 == 0)
		*sx = 0;
	else
		strcpy(sx, s2 + 2);
	
	return sx;
}

void dbg_print_device_info(device_t *dev)
{
	printf("%-10s %-10s ", dev->name, dev->type->name);
	// FIXME cmd_run_by_name("info", &pars_end, dev->type->cmds, dev);
}

/** Show statistics for specified device
 *
 */
void dbg_print_device_stat(device_t *dev)
{
	printf("%-10s %-10s ", dev->name, dev->type->name);
	// FIXME cmd_run_by_name("stat", &pars_end, dev->type->cmds, dev);
}

void dbg_print_devices(device_filter_t filter)
{
	// FIXME
	// device_t *device = NULL;
	// bool device_found = false;
	// 
	// printf(header);
	// 
	// while (dev_next(&device, filter)) {
	// 	device_found = true;
	// 	print_function(device);
	// }
	// 
	// if (!device_found)
	// 	printf(nothing_msg);
}

void dbg_print_devices_stat(device_filter_t filter)
{
	// FIXME
}
