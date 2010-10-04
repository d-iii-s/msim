/*
 * Copyright (c) 2001-2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Instruction decoding
 *
 */

#include "instr.h"

static instr_form_t instr_table[64] = {
	/* 0x00 */
	{ opcSPECIAL, ifREG },
	{ opcBCOND,   ifIMM },
	{ opcJ,       ifJ },
	{ opcJAL,     ifJ },
	{ opcBEQ,     ifIMM },
	{ opcBNE,     ifIMM },
	{ opcBLEZ,    ifIMM },
	{ opcBGTZ,    ifIMM },
	
	/* 0x08 */
	{ opcADDI,  ifIMM },
	{ opcADDIU, ifIMM },
	{ opcSLTI,  ifIMM },
	{ opcSLTIU, ifIMM },
	{ opcANDI,  ifIMM },
	{ opcORI,   ifIMM },
	{ opcXORI,  ifIMM },
	{ opcLUI,   ifIMM },
	
	/* 0x10 */
	{ opcCOP0,  ifREG },
	{ opcCOP1,  ifREG },
	{ opcCOP2,  ifREG },
	{ opcCOP3,  ifIMM },
	{ opcBEQL,  ifIMM },
	{ opcBNEL,  ifIMM },
	{ opcBLEZL, ifIMM },
	{ opcBGTZL, ifIMM },
	
	/* 0x18 */
	{ opcDADDI,    ifIMM },
	{ opcDADDIU,   ifIMM },
	{ opcLDL,      ifIMM },
	{ opcLDR,      ifIMM },
	{ opcSPECIAL2, ifREG },
	{ opcRES,      ifIMM },
	{ opcRES,      ifIMM },
	{ opcRES,      ifIMM },
	
	/* 0x20 */
	{ opcLB,  ifIMM },
	{ opcLH,  ifIMM },
	{ opcLWL, ifIMM },
	{ opcLW,  ifIMM },
	{ opcLBU, ifIMM },
	{ opcLHU, ifIMM },
	{ opcLWR, ifIMM },
	{ opcLWU, ifIMM },
	
	/* 0x28 */
	{ opcSB,    ifIMM },
	{ opcSH,    ifIMM },
	{ opcSWL,   ifIMM },
	{ opcSW,    ifIMM },
	{ opcSDL,   ifIMM },
	{ opcSDR,   ifIMM },
	{ opcSWR,   ifIMM },
	{ opcCACHE, ifIMM },
	
	/* 0x30 */
	{ opcLL,   ifIMM },
	{ opcLWC1, ifIMM },
	{ opcLWC2, ifIMM },
	{ opcRES,  ifIMM },
	{ opcLLD,  ifIMM },
	{ opcLDC1, ifIMM },
	{ opcLDC2, ifIMM },
	{ opcLD,   ifIMM },
	
	/* 0x38 */
	{ opcSC,   ifIMM },
	{ opcSWC1, ifIMM },
	{ opcSWC2, ifIMM },
	{ opcRES,  ifIMM },
	{ opcSCD,  ifIMM },
	{ opcSDC1, ifIMM },
	{ opcSDC2, ifIMM },
	{ opcSD,   ifIMM }
};

/** Sub-codes for SPECIAL code, all of there are ifREG */
static instr_opcode_t SPEC_instr_table[64] = {
	/* 0x00 */
	opcSLL,     opcRES,   opcSRL,  opcSRA,
	opcSLLV,    opcRES,   opcSRLV, opcSRAV,
	opcJR,      opcJALR,  opcMOVZ, opcMOVN,
	opcSYSCALL, opcBREAK, opcRES,  opcSYNC,
	
	/* 0x10 */
	opcMFHI,  opcMTHI,   opcMFLO,  opcMTLO,
	opcDSLLV, opcRES,    opcDSRLV, opcDSRAV,
	opcMULT,  opcMULTU,  opcDIV,   opcDIVU,
	opcDMULT, opcDMULTU, opcDDIV,  opcDDIVU,
	
	/* 0x20 */
	opcADD,  opcADDU,  opcSUB,  opcSUBU,
	opcAND,  opcOR,    opcXOR,  opcNOR,
	opcDHLT, opcDINT,  opcSLT,  opcSLTU,
	opcDADD, opcDADDU, opcDSUB, opcDSUBU,
	
	/* 0x30 */
	opcTGE,    opcTGEU, opcTLT,    opcTLTU,
	opcTEQ,    opcDVAL, opcTNE,    opcDRV,
	opcDSLL,   opcDTRC, opcDSRL,   opcDSRA,
	opcDSLL32, opcDTRO, opcDSRL32, opcDSRA32
};

/** Sub-codes for SPECIAL2 code */
static instr_opcode_t SPEC2_instr_table[64] = {
	/* 0x00 */
	opcMADD, opcMADDU, opcMUL, opcRES,
	opcMSUB, opcMSUBU, opcRES, opcRES,
	opcRES,  opcRES,   opcRES, opcRES,
	opcRES,  opcRES,   opcRES, opcRES,
	
	/* 0x10 */
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	
	/* 0x20 */
	opcCLZ, opcCLO, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	
	/* 0x30 */
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES,
	opcRES, opcRES, opcRES, opcRES
};

/** Sub-codes for COPz */
static instr_opcode_t COPz_spec[4][32] = {
	{
		/* 0x00 */
		opcMFC0, opcDMFC0, opcCFC0, opcRES,
		opcMTC0, opcDMTC0, opcCTC0, opcRES,
		opcBC,   opcRES,   opcRES,  opcRES,
		opcRES,  opcRES,   opcRES,  opcRES,
		
		/* 0x10 */
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0
	},
	{
		/* 0x00 */
		opcMFC1, opcDMFC1, opcCFC1, opcRES,
		opcMTC1, opcDMTC1, opcCTC1, opcRES,
		opcBC,   opcRES,   opcRES,  opcRES,
		opcRES,  opcRES,   opcRES,  opcRES,
		
		/* 0x10 */
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0
	},
	{
		/* 0x00 */
		opcMFC2, opcDMFC2, opcCFC2, opcRES,
		opcMTC2, opcDMTC2, opcCTC2, opcRES,
		opcBC,   opcRES,   opcRES,  opcRES,
		opcRES,  opcRES,   opcRES,  opcRES,
		
		/* 0x10 */
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0
	},
	{
		/* 0x00 */
		opcMFC3, opcDMFC3, opcCFC3, opcRES,
		opcMTC3, opcDMTC3, opcCTC3, opcRES,
		opcBC,   opcRES,   opcRES,  opcRES,
		opcRES,  opcRES,   opcRES,  opcRES,
		
		/* 0x10 */
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0,
		opcC0, opcC0, opcC0, opcC0
	}
};

/** Sub-codes for BCx */
static instr_opcode_t BCx_spec[4][32] = {
	{
		/* 0x00 */
		opcBC0F, opcBC0T, opcBC0FL, opcBC0TL,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		
		/* 0x10 */
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES
	},
	{
		/* 0x00 */
		opcBC1F, opcBC1T, opcBC1FL, opcBC1TL,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		
		/* 0x10 */
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES
	},
	{
		/* 0x00 */
		opcBC2F, opcBC2T, opcBC2FL, opcBC2TL,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		
		/* 0x10 */
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES
	},
	{
		/* 0x00 */
		opcBC3F, opcBC3T, opcBC3FL, opcBC3TL,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		opcRES,  opcRES,  opcRES,   opcRES,
		
		/* 0x10 */
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES,
		opcRES, opcRES, opcRES, opcRES
	}
};

/** Sub-codes for C0 */
static instr_opcode_t CO_spec[64] = {
	/* 0x00 */
	opcQRES, opcTLBR, opcTLBWI, opcQRES,
	opcQRES, opcQRES, opcTLBWR, opcQRES,
	opcTLBP, opcQRES, opcQRES,  opcQRES,
	opcQRES, opcQRES, opcQRES,  opcQRES,
	
	/* 0x10 */
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcERET, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	
	/* 0x20 */
	opcWAIT, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	
	/* 0x30 */
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES,
	opcQRES, opcQRES, opcQRES, opcQRES
};

/** Codes for regimm */
static instr_opcode_t reg_imm_instr_table[32] = {
	/* 0x00 */
	opcBLTZ, opcBGEZ,  opcBLTZL, opcBGEZL,
	opcRES,  opcRES,   opcRES,   opcRES,
	opcTGEI, opcTGEIU, opcTLTIU, opcTEQI,
	opcRES,  opcTNEI,  opcRES,   opcRES,
	
	/* 0x10 */
	opcBLTZAL, opcBGEZAL, opcBLTZALL, opcBGEZALL,
	opcRES,    opcRES,    opcRES,     opcRES,
	opcRES,    opcRES,    opcRES,     opcRES,
	opcRES,    opcRES,    opcRES,     opcRES
};

/** Instruction names */
instr_text_t instr_names_acronym[] = {
	/* Special names for blocks of instructions */
	{ "_spec",   ifNONE },
	{ "_branch", ifNONE },
	{ "_spec2",  ifNONE },
	
	/* Real instructions */
	{ "add",   ifREG },
	{ "addi",  ifIMMS },
	{ "addiu", ifIMMU },
	{ "addu",  ifREG },
	{ "and",   ifREG },
	{ "andi",  ifIMMUX },
	
	{ "bc0f",  ifOFF },
	{ "bc1f",  ifOFF },
	{ "bc2f",  ifOFF },
	{ "bc3f",  ifOFF },
	{ "bc0fl", ifOFF },
	{ "bc1fl", ifOFF },
	{ "bc2fl", ifOFF },
	{ "bc3fl", ifOFF },
	{ "bc0t",  ifOFF },
	{ "bc1t",  ifOFF },
	{ "bc2t",  ifOFF },
	{ "bc3t",  ifOFF },
	{ "bc0tl", ifOFF },
	{ "bc1tl", ifOFF },
	{ "bc2tl", ifOFF },
	{ "bc3tl", ifOFF },
	
	{ "beq",     ifCND },
	{ "beql",    ifCND },
	{ "bgez",    ifRO },
	{ "bgezal",  ifRO },
	{ "bgezall", ifRO },
	{ "bgezl",   ifRO },
	{ "bgtz",    ifRO },
	{ "bgtzl",   ifRO },
	{ "blez",    ifRO },
	{ "blezl",   ifRO },
	{ "bltz",    ifRO },
	{ "bltzal",  ifRO },
	{ "bltzall", ifRO },
	{ "bltzl",   ifRO },
	{ "bne",     ifCND },
	{ "bnel",    ifCND },
	{ "break",   ifNONE },
	
	{ "cache", ifNONE },
	{ "cfc0",  ifTD },
	{ "cfc1",  ifTD },
	{ "cfc2",  ifTD },
	{ "cfc3",  ifTD },
	{ "clo",   ifDS },
	{ "clz",   ifDS },
	{ "cop0",  ifOP },
	{ "cop1",  ifOP },
	{ "cop2",  ifOP },
	{ "cop3",  ifOP },
	{ "ctc0",  ifTD },
	{ "ctc1",  ifTD },
	{ "ctc2",  ifTD },
	{ "ctc3",  ifTD },
	
	{ "dadd",   ifR4 },
	{ "daddi",  ifR4 },
	{ "daddiu", ifR4 },
	{ "daddu",  ifR4 },
	{ "ddiv",   ifR4 },
	{ "ddivu",  ifR4 },
	{ "div",    ifST },
	{ "divu",   ifST },
	{ "dmfc0",  ifR4 },
	{ "dmfc1",  ifR4 },
	{ "dmfc2",  ifR4 },
	{ "dmfc3",  ifR4 },
	{ "dmtc0",  ifR4 },
	{ "dmtc1",  ifR4 },
	{ "dmtc2",  ifR4 },
	{ "dmtc3",  ifR4 },
	{ "dmult",  ifR4 },
	{ "dmultu", ifR4 },
	{ "dsll",   ifR4 },
	{ "dsllv",  ifR4 },
	{ "dsll32", ifR4 },
	{ "dsra",   ifR4 },
	{ "dsrav",  ifR4 },
	{ "dsra32", ifR4 },
	{ "dsrl",   ifR4 },
	{ "dsrlv",  ifR4 },
	{ "dsrl32", ifR4 },
	{ "dsub",   ifR4 },
	{ "dsubu",  ifR4 },
	
	{ "eret", ifNONE },
	
	{ "j",    ifJ },
	{ "jal",  ifJ },
	{ "jalr", ifDS },
	{ "jr",   ifS },
	
	{ "lb",   ifTOB },
	{ "lbu",  ifTOB },
	{ "ld",   ifTOB },
	{ "ldc1", ifTOB },
	{ "ldc2", ifTOB },
	{ "ldl",  ifTOB },
	{ "ldr",  ifTOB },
	{ "lh",   ifTOB },
	{ "lhu",  ifTOB },
	{ "ll",   ifTOB },
	{ "lld",  ifTOB },
	{ "lui",  ifRIW },
	{ "lw",   ifTOB },
	{ "lwc1", ifTOB },
	{ "lwc2", ifTOB },
	{ "lwl",  ifTOB },
	{ "lwr",  ifTOB },
	{ "lwu",  ifTOB },
	
	{ "madd",  ifST },
	{ "maddu", ifST },
	{ "mfc0",  ifTDX0 },
	{ "mfc1",  ifTDX1 },
	{ "mfc2",  ifTDX2 },
	{ "mfc3",  ifTDX3 },
	{ "mfhi",  ifD },
	{ "mflo",  ifD },
	{ "movn",  ifREG },
	{ "movz",  ifREG },
	{ "msub",  ifST },
	{ "msubu", ifST },
	{ "mtc0",  ifTDX0 },
	{ "mtc1",  ifTDX1 },
	{ "mtc2",  ifTDX2 },
	{ "mtc3",  ifTDX3 },
	{ "mthi",  ifS },
	{ "mtlo",  ifS },
	{ "mul",   ifREG },
	{ "mult",  ifST },
	{ "multu", ifST },
	
	{ "nor",   ifREG },
	
	{ "or",    ifREG },
	{ "ori",   ifIMMUX },
	
	{ "sb",      ifTOB },
	{ "sc",      ifTOB },
	{ "scd",     ifR4 },
	{ "sd",      ifR4 },
	{ "sdc1",    ifR4 },
	{ "sdc2",    ifR4 },
	{ "sdl",     ifR4 },
	{ "sdr",     ifR4 },
	{ "sh",      ifTOB },
	{ "sll",     ifDTS },
	{ "sllv",    ifREG },
	{ "slt",     ifREG },
	{ "slti",    ifIMMS },
	{ "sltiu",   ifIMMU },
	{ "sltu",    ifREG },
	{ "sra",     ifDTS },
	{ "srav",    ifREG },
	{ "srl",     ifDTS },
	{ "srlv",    ifREG },
	{ "sub",     ifREG },
	{ "subu",    ifREG },
	{ "sw",      ifTOB },
	{ "swc1",    ifTOB },
	{ "swc2",    ifTOB },
	{ "swl",     ifTOB },
	{ "swr",     ifTOB },
	{ "sync",    ifNONE },
	{ "syscall", ifSYSCALL },
	
	{ "teq",   ifST },
	{ "teqi",  ifSI },
	{ "tge",   ifST },
	{ "tgei",  ifSI },
	{ "tgeiu", ifSIW },
	{ "tgeu",  ifST },
	{ "tlbp",  ifNONE },
	{ "tlbr",  ifNONE },
	{ "tlbwi", ifNONE },
	{ "tlbwr", ifNONE },
	{ "tlt",   ifST },
	{ "tlti",  ifSI },
	{ "tltiu", ifSIW },
	{ "tltu",  ifST },
	{ "tne",   ifST },
	{ "tnei",  ifSI },
	
	{ "wait", ifNONE },
	
	{ "xor",  ifREG },
	{ "xori", ifIMMUX },
	
	{ "nop", ifNONE },
	
	{ "unimp", ifERR },
	
	{ "res",  ifERR },
	{ "qres", ifERR },
	
	{ "d_value",    ifERR },
	{ "d_trace",    ifERR },
	{ "d_traceoff", ifERR },
	{ "d_regview",  ifERR },
	{ "d_halt",     ifERR },
	
	{ "---", ifERR }
};

char *reg_name[][32] = {
	{
		"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",
		"r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
		"r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
		"r30", "r31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"0",  "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1",
		"t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", 
		"s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp",
		"fp", "ra"
	}
};

char *cp0_name[][32] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"index",    "random",   "entrylo0", "entrylo1",
		"context",  "pagemask", "wired",    "res_7",
		"badvaddr", "count",    "entryhi",  "compare",
		"status",   "cause",    "epc",      "prid",
		"config",   "lladdr",   "watchlo",  "watchhi",
		"xcontext", "res_21",   "res_22",   "res_23",
		"res_24",   "res_25",   "res_26",   "res_27",
		"res_28",   "res_29",   "errorepc", "res_31"
	}
};

char *cp1_name[][32] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp1_0",  "cp1_1",  "cp1_2",  "cp1_3",  "cp1_4",  "cp1_5",  "cp1_6",
		"cp1_7",  "cp1_8",  "cp1_9",  "cp1_10", "cp1_11", "cp1_12", "cp1_13",
		"cp1_14", "cp1_15", "cp1_16", "cp1_17", "cp1_18", "cp1_19", "cp1_20",
		"cp1_21", "cp1_22", "cp1_23", "cp1_24", "cp1_25", "cp1_26", "cp1_27",
		"cp1_28", "cp1_29", "cp1_30", "cp1_31"
	}
};

char *cp2_name[][32] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp2_0",  "cp2_1",  "cp2_2",  "cp2_3",  "cp2_4",  "cp2_5",  "cp2_6",
		"cp2_7",  "cp2_8",  "cp2_9",  "cp2_10", "cp2_11", "cp2_12", "cp2_13",
		"cp2_14", "cp2_15", "cp2_16", "cp2_17", "cp2_18", "cp2_19", "cp2_20",
		"cp2_21", "cp2_22", "cp2_23", "cp2_24", "cp2_25", "cp2_26", "cp2_27",
		"cp2_28", "cp2_29", "cp2_30", "cp2_31"
	}
};

char *cp3_name[][32] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp3_0",  "cp3_1",  "cp3_2",  "cp3_3",  "cp3_4",  "cp3_5",  "cp3_6",
		"cp3_7",  "cp3_8",  "cp3_9",  "cp3_10", "cp3_11", "cp3_12", "cp3_13",
		"cp3_14", "cp3_15", "cp3_16", "cp3_17", "cp3_18", "cp3_19", "cp3_20",
		"cp3_21", "cp3_22", "cp3_23", "cp3_24", "cp3_25", "cp3_26", "cp3_27",
		"cp3_28", "cp3_29", "cp3_30", "cp3_31"
	}
};

static instr_opcode_t opcode_COPx(instr_opcode_t opc, uint32_t icode)
{
	instr_opcode_t opcode =
	    COPz_spec[opc - opcCOP0][(icode & RS_MASK) >> RS_SHIFT];
	
	switch (opcode) {
	case opcBC:
		opcode =
		    BCx_spec[opc - opcCOP0][(icode & RT_MASK) >> RT_SHIFT];
		break;
	case opcC0:
		opcode = CO_spec[icode & 0x3f];
		break;
	default:
		break;
	}
	
	return opcode;
}

/** Decode instruction
 *
 */
void decode_instr(instr_info_t *ii)
{
	/* Instruction name and type */
	instr_form_t *opc = &instr_table[(ii->icode >> 26) & 0x3f];
	ii->opcode = opc->opcode;
	
	switch (opc->opcode) {
	case opcSPECIAL:
		if (ii->icode == 0)
			ii->opcode = opcNOP;
		else
			ii->opcode = SPEC_instr_table[ii->icode & FUNCTION_MASK];
		
		ii->shift = (ii->icode >> 6) & 0x1f;
		break;
	case opcSPECIAL2:
		ii->opcode = SPEC2_instr_table[ii->icode & FUNCTION_MASK];
		ii->shift = (ii->icode >> 6) & 0x1f;
		break;
	case opcBCOND:
		ii->opcode = reg_imm_instr_table[(ii->icode >> 16) & 0x1f];
		break;
	case opcCOP0:
	case opcCOP1:
	case opcCOP2:
	case opcCOP3:
		ii->opcode = opcode_COPx(ii->opcode, ii->icode);
		break;
	default:
		break;
	}
	
	/* Parse parameters */
	switch (opc->format) {
	case ifIMM:
		/* Registers */
		ii->rs = (ii->icode & RS_MASK) >> RS_SHIFT;
		ii->rt = (ii->icode & RT_MASK) >> RT_SHIFT;
		ii->rd = 0;
		
		/* Immediate */
		ii->imm = ii->icode & IMM_MASK;
		if (ii->imm & IMM_SIGN_BIT)
			ii->imm |= ~((uint32_t) (IMM_MASK));
		break;
	case ifJ:
		ii->imm = ii->icode & TARGET_MASK;
		ii->rt = 0;
		ii->rs = 0;
		ii->rd = 0;
		break;
	case ifREG:
		/* Registers */
		ii->rs = (ii->icode & RS_MASK) >> RS_SHIFT;
		ii->rt = (ii->icode & RT_MASK) >> RT_SHIFT;
		ii->rd = (ii->icode & RD_MASK) >> RD_SHIFT;
		ii->sa = (ii->icode & SA_MASK) >> SA_SHIFT;
		ii->function = (ii->icode & FUNCTION_MASK);
	default:
		break;
	}
}
