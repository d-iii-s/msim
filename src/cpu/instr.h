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

#ifndef INSTR_H_
#define INSTR_H_

#include <stdint.h>
#include <inttypes.h>
#include "../../config.h"
#include "../main.h"

#define REG_COUNT     32
#define REG_VARIANTS  3

/** Opcode numbers
 *
 */
typedef enum {
	/* 0 */
	opcSPECIAL = 0,
	opcREGIMM = 1,
	opcJ = 2,
	opcJAL = 3,
	opcBEQ = 4,
	opcBNE = 5,
	opcBLEZ = 6,
	opcBGTZ = 7,
	
	/* 8 */
	opcADDI = 8,
	opcADDIU = 9,
	opcSLTI = 10,
	opcSLTIU = 11,
	opcANDI = 12,
	opcORI = 13,
	opcXORi = 14,
	opcLUI = 15,
	
	/* 16 */
	opcCOP0 = 16,
	opcCOP1 = 17,
	opcCOP2 = 18,
	/* opcode 19 unused */
	opcBEQL = 20,
	opcBNEL = 21,
	opcBLEZL = 22,
	opcBGTZL = 23,
	
	/* 24 */
	opcDADDI = 24,
	opcDADDIU = 25,
	opcLDL = 26,
	opcLDR = 27,
	/* opcode 28 unused */
	/* opcode 29 unused */
	/* opcode 30 unused */
	/* opcode 31 unused */
	
	/* 32 */
	opcLB = 32,
	opcLH = 33,
	opcLWL = 34,
	opcLW = 35,
	opcLBU = 36,
	opcLHU = 37,
	opcLWR = 38,
	opcLWU = 39,
	
	/* 40 */
	opcSB = 40,
	opcSH = 41,
	opcSWL = 42,
	opcSW = 43,
	opcSDL = 44,
	opcSDR = 45,
	opcSWR = 46,
	opcCACHE = 47,
	
	/* 48 */
	opcLL = 48,
	opcLWC1 = 49,
	opcLWC2 = 50,
	/* opcode 51 unused */
	opcLDD = 52,
	opcLDC1 = 53,
	opcLDC2 = 54,
	opcLD = 55,
	
	/* 56 */
	opcSC = 56,
	opcSWC1 = 57,
	opcSWC2 = 58,
	/* opcode 59 unused */
	opcSCD = 60,
	opcSDC1 = 61,
	opcSDC2 = 62,
	opcSD = 63
} instr_opcode_t;

/** Function numbers
 *
 * For opcSPECIAL instructions.
 *
 */
typedef enum {
	/* 0 */
	funcSLL = 0,
	/* function 1 unused */
	funcSRL = 2,
	funcSRA = 3,
	funcSLLV = 4,
	/* function 5 unused */
	funcSRLV = 6,
	funcSRAV = 7,
	
	/* 8 */
	funcJR = 8,
	funcJALR = 9,
	/* function 10 unused */
	/* function 11 unused */
	funcSYSCALL = 12,
	funcBREAK = 13,
	/* function 14 unused */
	funcSYNC = 15,
	
	/* 16 */
	funcMFHI = 16,
	funcMTHI = 17,
	funcMFLO = 18,
	funcMTLO = 19,
	funcDSLLV = 20,
	/* function 21 unused */
	funcDSRLV = 22,
	funcDSRAV = 23,
	
	/* 24 */
	funcMULT = 24,
	funcMULTU = 25,
	funcDIV = 26,
	funcDIVU = 27,
	funcDMULT = 28,
	funcDMULTU = 29,
	funcDDIV = 30,
	funcDDIVU = 31,
	
	/* 32 */
	funcADD = 32,
	funcADDU = 33,
	funcSUB = 34,
	funcSUBU = 35,
	funcAND = 36,
	funcOR = 37,
	funcXOR = 38,
	funcNOR = 39,
	
	/* 40 */
	/* function 40 unused */
	func_XINT = 41,
	funcSLT = 42,
	funcSLTU = 43,
	funcDADD = 44,
	funcDADDU = 45,
	funcDSUB = 46,
	funcDSUBu = 47,
	
	/* 48 */
	funcTGE = 48,
	funcTGEU = 49,
	funcTLT = 50,
	funcTLTU = 51,
	funcTEQ = 52,
	/* function 53 unused */
	funcTNE = 54,
	/* function 55 unused */
	
	/* 56 */
	funcDSLL = 56,
	/* function 57 unused */
	funcDSRL = 58,
	funcDSRA = 59,
	funcSLL32 = 60,
	/* function 61 unused */
	funcDSRL32 = 62,
	funcDSRA32 = 63
} instr_func_t;

/** Register rt numbers
 *
 * For opcREGIMM instructions.
 *
 */
typedef enum {
	/* 0 */
	rtBLTZ = 0,
	rtBGEZ = 1,
	rtBLTZL = 2,
	rtBGEZL = 3,
	/* rt 4 unused */
	/* rt 5 unused */
	/* rt 6 unused */
	/* rt 7 unused */
	
	/* 8 */
	rtTGEI = 8,
	rtTGEIU = 9,
	rtTLTI = 10,
	rtTLTIU = 11,
	rtTEQI = 12,
	/* rt 13 unused */
	rtTNEI = 14,
	/* rt 15 unused */
	
	/* 16 */
	rtBLTZAL = 16,
	rtBGEZAL = 17,
	rtBLTZALL = 18,
	rtBGEZALL = 19
} instr_rt_t;

typedef enum {
	/* 0 */
	cop0rsMFC0 = 0,
	cop0rsDMFC0 = 1,
	/* rs 2 unused */
	/* rs 3 unused */
	cop0rsMTC0 = 4,
	cop0rsDMTC0 = 5,
	/* rs 6 unused */
	/* rs 7 unused */
	
	/* 8 */
	cop0rsBC = 8,
	/* rs 9 unused */
	/* rs 10 unused */
	/* rs 11 unused */
	/* rs 12 unused */
	/* rs 13 unused */
	/* rs 14 unused */
	/* rs 15 unused */
	
	/* 16 */
	cop0rsCO = 16
} instr_cop0rs_t;

typedef enum {
	/* 0 */
	cop1rsMFC1 = 0,
	cop1rsDMFC1 = 1,
	cop1rsCFC1 = 2,
	/* rs 3 unused */
	cop1rsMTC1 = 4,
	cop1rsDMTC1 = 5,
	cop1rsCTC1 = 6,
	/* rs 7 unused */
	
	/* 8 */
	cop1rsBC = 8
} instr_cop1rs_t;

typedef enum {
	/* 0 */
	cop2rsMFC2 = 0,
	/* rs 1 unused */
	cop2rsCFC2 = 2,
	/* rs 3 unused */
	/* rs 4 unused */
	/* rs 5 unused */
	cop2rsCTC2 = 6,
	/* rs 7 unused */
	
	/* 8 */
	cop2rsBC = 8
} instr_cop2rs_t;

typedef enum {
	/* 0 */
	cop0rtBC0F = 0,
	cop0rtBC0T = 1,
	cop0rtBC0FL = 2,
	cop0rtBC0TL = 3
} instr_cop0rt_t;

typedef enum {
	/* 0 */
	cop1rtBC1F = 0,
	cop1rtBC1T = 1,
	cop1rtBC1FL = 2,
	cop1rtBC1TL = 3
} instr_cop1rt_t;

typedef enum {
	/* 0 */
	cop2rtBC2F = 0,
	cop2rtBC2T = 1,
	cop2rtBC2FL = 2,
	cop2rtBC2TL = 3
} instr_cop2rt_t;

typedef enum {
	/* 0 */
	/* function 0 unused */
	cop0funcTLBR = 1,
	cop0funcTLBWI = 2,
	/* function 3 unused */
	/* function 4 unused */
	/* function 5 unused */
	cop0funcTLBWR = 6,
	/* function 7 unused */
	
	/* 8 */
	cop0funcTLBP = 8,
	/* function 9 unused */
	/* function 10 unused */
	/* function 11 unused */
	/* function 12 unused */
	/* function 13 unused */
	/* function 14 unused */
	/* function 15 unused */
	
	/* 16 */
	cop0funcERET = 16
} instr_cop0func_t;

typedef union {
	uint32_t val;
#ifdef WORDS_BIGENDIAN
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int imm : 16;
	} i;
	struct {
		unsigned int opcode : 6;
		unsigned int target : 26;
	} j;
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int sa : 5;
		unsigned int func : 6;
	} r;
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int data : 24;
		unsigned int func : 8;
	} cop;
	struct {
		unsigned int opcode : 6;
		unsigned int code : 20;
		unsigned int func : 6;
	} sys;
#else
	struct {
		unsigned int imm : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} i;
	struct {
		unsigned int target : 26;
		unsigned int opcode : 6;
	} j;
	struct {
		unsigned int func : 6;
		unsigned int sa : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} r;
	struct {
		unsigned int func : 8;
		unsigned int data : 24;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} cop;
	struct {
		unsigned int func : 6;
		unsigned int code : 20;
		unsigned int opcode : 6;
	} sys;
#endif
} instr_t;

/** Register and coprocessor names */
extern char *reg_name[REG_VARIANTS][REG_COUNT];
extern char *cp0_name[REG_VARIANTS][REG_COUNT];
extern char *cp1_name[REG_VARIANTS][REG_COUNT];
extern char *cp2_name[REG_VARIANTS][REG_COUNT];
extern char *cp3_name[REG_VARIANTS][REG_COUNT];

#endif
