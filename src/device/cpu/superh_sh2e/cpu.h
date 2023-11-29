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

#ifndef SUPERH_SH2E_CPU_H_
#define SUPERH_SH2E_CPU_H_

#include "insn.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "../../../assert.h"
#include "../../../list.h"
#include "../../../physmem.h"
#include "../../../utils.h"

#define PACKED __attribute__((packed))

#define SH2E_GENERAL_REG_COUNT 16
#define SH2E_CONTROL_REG_COUNT 3
#define SH2E_SYSTEM_REG_COUNT 4
#define SH2E_FPU_GENERAL_REG_COUNT 16
#define SH2E_FPU_SYSTEM_REG_COUNT 4

typedef enum {
    SH2E_BRANCH_STATE_NONE = 0,
    SH2E_BRANCH_STATE_DELAY,
    SH2E_BRANCH_STATE_EXECUTE,
} sh2e_branch_state_t;

/** Processor data types. */
typedef float float32_t;

typedef union {
    uint32_t ivalue;
    float32_t fvalue;
    struct {
#ifdef WORDS_BIGENDIAN
        uint32_t sign : 1;
        uint32_t exponent : 8;
        uint32_t mantissa : 23;
#else
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
#endif
    } PACKED;
} float32_bits_t;



typedef enum sh2e_processing_state {
    SH2E_PSTATE_POWER_ON_RESET,
    SH2E_PSTATE_EXCEPTION_PROCESSING,
    SH2E_PSTATE_BUS_RELEASE,
    SH2E_PSTATE_PROGRAM_EXECUTION,
    SH2E_PSTATE_POWER_DOWN,
    __SH2E_PSTATE_COUNT,
} sh2e_processing_state_t;


struct frame;

typedef union sh2e_cpu_sr {
    uint32_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint32_t _rf1 : 22; /** Reserved field 1, bits 31 to 10. */
        uint32_t m : 1; /** M bit for DIV0U/S and DIV1 instructions. */
        uint32_t q : 1; /** Q bit for DIV0U/S and DIV1 instructions. */
        uint32_t im : 4; /** Interrupt mask. */
        uint32_t _rf0 : 2; /** Reserved field 0, bits 3 to 2. */
        uint32_t s : 1; /** S bit for MAC instructions. */
        uint32_t t : 1; /** T bit for conditional instructions. */
#else
        uint32_t t : 1; /** T bit for conditional instructions. */
        uint32_t s : 1; /** S bit for MAC instructions. */
        uint32_t _rf0 : 2; /** Reserved field 0, bits 3 to 2. */
        uint32_t im : 4; /** Interrupt mask. */
        uint32_t q : 1; /** Q bit for DIV0U/S and DIV1 instructions. */
        uint32_t m : 1; /** M bit for DIV0U/S and DIV1 instructions. */
        uint32_t _rf1 : 22; /** Reserved field 1, bits 31 to 10. */
#endif
    };
} sh2e_cpu_sr_t;

_Static_assert(sizeof(sh2e_cpu_sr_t) == sizeof(uint32_t), "invalid sh2e_cpu_sr_t size!");


typedef struct sh2e_cpu_regs {
    /* General registers. */

    union {
        uint32_t general[SH2E_GENERAL_REG_COUNT];

        PACKED struct {
            uint32_t r0;
            uint32_t r1;
            uint32_t r2;
            uint32_t r3;
            uint32_t r4;
            uint32_t r5;
            uint32_t r6;
            uint32_t r7;
            uint32_t r8;
            uint32_t r9;
            uint32_t r10;
            uint32_t r11;
            uint32_t r12;
            uint32_t r13;
            uint32_t r14;
            uint32_t sp;
        };
    };


    /* Control registers. */

    union {
        uint32_t control[SH2E_CONTROL_REG_COUNT];

        PACKED struct {
            /** Status register. */
            sh2e_cpu_sr_t sr;

            /**
             * Global Base Register.
             *
             * Indicates the base address of the indirect GBR addressing mode.
             * The indirect GBR addressing mode is used in data transfer for
             * on-chip peripheral module register areas and in logic operations.
             */
            uint32_t gbr;

            /**
             * Vector Base Register.
             *
             * Indicates the base address of the exception processing vector area.
             */
            uint32_t vbr;
        };
    };


    /* System registers. */

    union {
        uint32_t system[SH2E_SYSTEM_REG_COUNT];

        PACKED struct {
            /** Multiply-and-accumulate register high. */
            uint32_t mach;

            /** Multiply-and-accumulate register low. */
            uint32_t macl;

            /**
             * Procedure register.
             *
             * Stores the return address from a subroutine/procedure.
             */
            uint32_t pr;

            /** Program counter. */
            uint32_t pc;
        };
    };
} sh2e_cpu_regs_t;


typedef union sh2e_fpu_scr {
    uint32_t value;

    PACKED struct {
        uint32_t _rf0 : 13; /** Reserved field 0 (bits 31 to 19). Wired to 0. */

        uint32_t dn : 1; /** Denormalized bit. Wired to 1 on SH-2E. */

        uint32_t ce : 1; /** FPU error cause bit. Wired to 0 on SH-2E. */

        uint32_t cv : 1; /** Invalid operation cause bit. */
        uint32_t cz : 1; /** Division-by-zero cause bit. */
        uint32_t co : 1; /** Overflow cause bit. Wired to 0 on SH-2E. */
        uint32_t cu : 1; /** Underflow cause bit. Wired to 0 on SH-2E. */
        uint32_t ci : 1; /** Inexact cause bit. Wired to 0 on SH-2E. */

        uint32_t ev : 1; /** Invalid operation exception enable. */
        uint32_t ez : 1; /** Division-by-zero exception enable. */
        uint32_t eo : 1; /** Overflow exception enable. Wired to 0 on SH-2E. */
        uint32_t eu : 1; /** Underflow exception enable. Wired to 0 on SH-2E. */
        uint32_t ei : 1; /** Inexact exception enable. Wired to 0 on SH-2E. */

        uint32_t fv : 1; /** Invalid operation flag. */
        uint32_t fz : 1; /** Division-by-zero flat. */
        uint32_t fo : 1; /** Overflow exception flag. Wired to 0 on SH-2E. */
        uint32_t fu : 1; /** Underflow exception flag. Wired to 0 on SH-2E. */
        uint32_t fi : 1; /** Inexact exception flag. Wired to 0 on SH-2E. */

        uint32_t rm : 2; /** Rounding Mode. Wired to 0b01 (rounding to zero) on SH-2E. */
    };
} sh2e_fpu_scr_t;

_Static_assert(sizeof(sh2e_fpu_scr_t) == sizeof(uint32_t), "invalid sh2e_fpu_scr_t size!");


typedef float32_bits_t sh2e_fpu_ul_t;
_Static_assert(sizeof(sh2e_fpu_ul_t) == sizeof(uint32_t), "invalid sh2e_fpu_ul_t size!");

typedef struct sh2e_fpu_regs {

    /* General registers. */

    union {
        float32_t general[SH2E_FPU_GENERAL_REG_COUNT];

        PACKED struct {
            float32_t fr0;
            float32_t fr1;
            float32_t fr2;
            float32_t fr3;
            float32_t fr4;
            float32_t fr5;
            float32_t fr6;
            float32_t fr7;
            float32_t fr8;
            float32_t fr9;
            float32_t fr10;
            float32_t fr11;
            float32_t fr12;
            float32_t fr13;
            float32_t fr14;
            float32_t fr15;
        };
    };

    /* System registers. */

    union {
        uint32_t system[SH2E_FPU_SYSTEM_REG_COUNT];

        PACKED struct {
            uint32_t _rsv0;

            /**
             * Floating point communication register.
             *
             * Used for communication between the CPU and the FPU.
             */
            sh2e_fpu_ul_t fpul;

            /**
             * Floating-point status/control register.
             *
             * Indicates and stores status/control information
             * relating to FPU exceptions.
             */
            sh2e_fpu_scr_t fpscr;

            uint32_t _rsv3;
        };
    };

} sh2e_fpu_regs_t;


/** Main processor structure */
typedef struct sh2e_cpu {

    sh2e_cpu_regs_t cpu_regs;

    sh2e_fpu_regs_t fpu_regs;

    /* Execution state. */

    /** The next value of PC. Used to implement jumps, branches, and traps. */
    uint32_t pc_next;

    sh2e_processing_state_t pr_state;

    sh2e_branch_state_t br_state;

    /** Accounting. */
    uint_fast64_t cycles;

    unsigned int id;
} sh2e_cpu_t;



/** Exception codes. */
typedef enum sh2e_exception {
    SH2E_EXCEPTION_CPU_ADDRESS_ERROR,
    SH2E_EXCEPTION_ILLEGAL_INSTRUCTION,
    SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION,
    SH2E_EXCEPTION_FPU_OPERATION,
    __SH2E_EXCEPTION_COUNT,
    SH2E_EXCEPTION_NONE = __SH2E_EXCEPTION_COUNT,
} sh2e_exception_t;

/** Instruction implementation. */
typedef sh2e_exception_t (*sh2e_insn_exec_fn_t)(sh2e_cpu_t * cpu, sh2e_insn_t insn);


/** Memory read implementation. */
typedef sh2e_exception_t (*sh2e_cpu_read_fn_t)(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);


/** Memory write implementation. */
typedef sh2e_exception_t (*sh2e_cpu_write_fn_t)(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t value);


/** Operand extension implementation. */
typedef uint32_t (*sh2e_extend_fn_t)(uint_fast32_t value);


/* Exception processing Vector Area */

#define SH2E_EVA_VECTOR_COUNT 256

typedef struct sh2e_eva {
    union {
        uint32_t vectors[SH2E_EVA_VECTOR_COUNT];

        PACKED struct {
            uint32_t power_on_reset_pc;
            uint32_t power_on_reset_sp;

            uint32_t manual_reset_pc;
            uint32_t manual_reset_sp;

            uint32_t general_illegal_insn;
            uint32_t __reserved_5;

            uint32_t slot_illegal_insn;
            uint32_t __reserved_7;
            uint32_t __reserved_8;

            uint32_t cpu_address_error;
            uint32_t dmac_address_error;

            uint32_t nmi_interrupt;
            uint32_t user_break_interrupt;
            uint32_t fpu_exception;
            uint32_t hudi_interrupt;

            uint32_t __reserved_15_31[17];

            uint32_t trap[32];

            union {
                uint32_t irq[8];

                PACKED struct {
                    uint32_t irq0;
                    uint32_t irq1;
                    uint32_t irq2;
                    uint32_t irq3;
                    uint32_t irq4;
                    uint32_t irq5;
                    uint32_t irq6;
                    uint32_t irq7;
                };
            };

            uint32_t on_chip_peripheral[183];
        };
    };
} sh2e_eva_t;

_Static_assert(sizeof(sh2e_eva_t) == SH2E_EVA_VECTOR_COUNT * sizeof(uint32_t), "invalid sh2e_epv_area_t size!");


/** Basic CPU routines */
extern void sh2e_cpu_init(sh2e_cpu_t * cpu, unsigned int id);
extern void sh2e_cpu_done(sh2e_cpu_t * cpu);
extern void sh2e_cpu_step(sh2e_cpu_t * cpu);
extern void sh2e_cpu_goto(sh2e_cpu_t * cpu, ptr64_t addr);


/** Memory operations */
extern bool sh2e_cpu_convert_addr(sh2e_cpu_t const * cpu, ptr64_t virt, ptr36_t * phys, bool write);

// extern sh2e_exception_t sh2e_cpu_readz_byte(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);
// extern sh2e_exception_t sh2e_cpu_reads_byte(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);

// extern sh2e_exception_t sh2e_cpu_readz_word(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);
// extern sh2e_exception_t sh2e_cpu_reads_word(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);

extern sh2e_exception_t sh2e_cpu_fetch_insn(sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t * output_insn);
// extern sh2e_exception_t sh2e_cpu_read_long(sh2e_cpu_t const * cpu, uint32_t addr, uint32_t * output_value);

/** Interrupts */
extern void sh2e_cpu_assert_interrupt(sh2e_cpu_t * cpu, unsigned int num);
extern void sh2e_cpu_deassert_interrupt(sh2e_cpu_t * cpu, unsigned int num);

#endif // SUPERH_SH2E_CPU_H_
