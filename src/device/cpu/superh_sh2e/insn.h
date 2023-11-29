/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */


#ifndef SUPERH_SH2E_INSN_H_
#define SUPERH_SH2E_INSN_H_

#include "../../../../config.h"

#include <stdint.h>

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif


/****************************************************************************
 * SH-2E instruction formats
 ****************************************************************************/

// 0-format
//
// ic ... instruction code
//
typedef struct {
    uint16_t ic : 16;
} PACKED sh2e_insn_z_t;

_Static_assert(sizeof(sh2e_insn_z_t) == sizeof(uint16_t), "invalid sh2e_insn_z_t size!");


// n-format
//
// ic_h:ic_l ... instruction code
// rn        ... destination register
//
// Direct register addressing.
// Direct register addressing (store with control or system registers).
// Indirect register addressing.
// Pre-decrement indirect register addressing.
// Floating-point instruction.
//
typedef union {
    uint16_t word;
    struct {
#ifdef WORDS_BIGENDIAN
        uint16_t ic_h : 4;
        uint16_t rn : 4;
        uint16_t ic_l : 8;
#else
        uint16_t ic_l : 8;
        uint16_t rn : 4;
        uint16_t ic_h : 4;
#endif
    } PACKED;
} sh2e_insn_n_t;

_Static_assert(sizeof(sh2e_insn_n_t) == sizeof(uint16_t), "invalid sh2e_insn_n_t size!");


// m-format
//
// ic_h:ic_l ... instruction code
// rm        ... source register
//
// Direct register addressing (load with control or system registers).
// PC relative addressing with Rm.
// Indirect register addressing.
// Post-increment indirect register addressing.
// Floating-point instruction.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic_h : 4;
    uint16_t rm : 4;
    uint16_t ic_l : 8;
#else
    uint16_t ic_l : 8;
    uint16_t rm : 4;
    uint16_t ic_h : 4;
#endif
} PACKED sh2e_insn_m_t;

_Static_assert(sizeof(sh2e_insn_m_t) == sizeof(uint16_t), "invalid sh2e_insn_m_t size!");


// nm-format
//
// ic_h:ic_l ... instruction code
// rn        ... destination register
// rm        ... source register
//
// Direct register addressing (load with control or system registers).
// PC relative addressing with Rm.
// Indirect register addressing.
// Post-increment indirect register addressing.
// Floating-point instruction.
//
typedef union {
    uint16_t word;
    struct {
#ifdef WORDS_BIGENDIAN
        uint16_t ic_h : 4;
        uint16_t rn : 4;
        uint16_t rm : 4;
        uint16_t ic_l : 4;
#else
        uint16_t ic_l : 4;
        uint16_t rm : 4;
        uint16_t rn : 4;
        uint16_t ic_h : 4;
#endif
    } PACKED;
} sh2e_insn_nm_t;

_Static_assert(sizeof(sh2e_insn_nm_t) == sizeof(uint16_t), "invalid sh2e_insn_nm_t size!");


// md-format
//
// ic ... instruction code
// rm ... displacement
// d4 ... 4-bit immediate displacement
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 8;
    uint16_t rm : 4;
    uint16_t d4 : 4;
#else
    uint16_t d4 : 4;
    uint16_t rm : 4;
    uint16_t ic : 8;
#endif
} PACKED sh2e_insn_md_t;

_Static_assert(sizeof(sh2e_insn_md_t) == sizeof(uint16_t), "invalid sh2e_insn_md_t size!");


// nd4-format
//
// ic ... instruction code
// rn ... destination register
// d4 ... 4-bit immediate displacement
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    // R0 source
    uint16_t ic : 8;
    uint16_t rn : 4;
    uint16_t d4 : 4;
#else
    uint16_t d4 : 4;
    uint16_t rn : 4;
    uint16_t ic : 8;
#endif
} PACKED sh2e_insn_nd4_t;

_Static_assert(sizeof(sh2e_insn_nd4_t) == sizeof(uint16_t), "invalid sh2e_insn_nd4_t size!");


// nmd-format
//
// ic ... instruction code
// rn ... destination register
// d4 ... 4-bit immediate displacement
// rm ... source register
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 4;
    uint16_t rn : 4;
    uint16_t d4 : 4;
    uint16_t rm : 4;
#else
    uint16_t rm : 4;
    uint16_t d4 : 4;
    uint16_t rn : 4;
    uint16_t ic : 4;
#endif
} PACKED sh2e_insn_nmd_t;

_Static_assert(sizeof(sh2e_insn_nmd_t) == sizeof(uint16_t), "invalid sh2e_insn_nmd_t size!");


// d-format
//
// ic ... instruction code
// d8 ... 8-bit immediate displacement
//
// Indirect GBR addressing with displacement.
// Indirect PC addressing with displacement.
// PC relative addressing.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    // R0 source
    uint16_t ic : 8;
    uint16_t d8 : 8;
#else
    uint16_t d8 : 8;
    uint16_t ic : 8;
#endif
} PACKED sh2e_insn_d_t;

_Static_assert(sizeof(sh2e_insn_d_t) == sizeof(uint16_t), "invalid sh2e_insn_d_t size!");


// d12-format
//
// ic  ... instruction code
// d12 ... 12-bit immediate displacement
//
// PC relative addressing.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 4;
    uint16_t d12 : 12;
#else
    uint16_t d12 : 12;
    uint16_t ic : 4;
#endif
} PACKED sh2e_insn_d12_t;

_Static_assert(sizeof(sh2e_insn_d12_t) == sizeof(uint16_t), "invalid sh2e_insn_d12_t size!");


// nd8-format
//
// ic ... instruction code
// rn ... destination register
// d8 ... 8-bit immediate displacement
//
// PC relative addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 4;
    uint16_t rn : 4;
    uint16_t d8 : 8;
#else
    uint16_t d8 : 8;
    uint16_t rn : 4;
    uint16_t ic : 4;
#endif
} PACKED sh2e_insn_nd8_t;

_Static_assert(sizeof(sh2e_insn_nd8_t) == sizeof(uint16_t), "invalid sh2e_insn_nd8_t size!");


// i-format
//
// ic ... instruction code
// i8 ... 8-bit immediate
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 8;
    uint16_t i8 : 8;
#else
    uint16_t i8 : 8;
    uint16_t ic : 8;
#endif
} PACKED sh2e_insn_i_t;

_Static_assert(sizeof(sh2e_insn_i_t) == sizeof(uint16_t), "invalid sh2e_insn_i_t size!");


// ni-format
//
// ic ... instruction code
// rn ... destination register
// i8 ... 8-bit immediate
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    uint16_t ic : 4;
    uint16_t rn : 4;
    uint16_t i8 : 8;
#else
    uint16_t i8 : 8;
    uint16_t rn : 4;
    uint16_t ic : 4;
#endif
} PACKED sh2e_insn_ni_t;

_Static_assert(sizeof(sh2e_insn_ni_t) == sizeof(uint16_t), "invalid sh2e_insn_ni_t size!");


/****************************************************************************
 * SH-2E instruction
 ****************************************************************************/

typedef union sh2e_insn {
    uint16_t word;

    sh2e_insn_z_t z_form;
    sh2e_insn_n_t n_form;
    sh2e_insn_m_t m_form;
    sh2e_insn_nm_t nm_form;
    sh2e_insn_md_t md_form;
    sh2e_insn_nd4_t nd4_form;
    sh2e_insn_nmd_t nmd_form;
    sh2e_insn_d_t d_form;
    sh2e_insn_d12_t d12_form;
    sh2e_insn_nd8_t nd8_form;
    sh2e_insn_i_t i_form;
    sh2e_insn_ni_t ni_form;
} sh2e_insn_t;

_Static_assert(sizeof(sh2e_insn_t) == sizeof(uint16_t), "invalid sh2e_insn_t size!");

#endif // SUPERH_SH2E_INSN_H_
