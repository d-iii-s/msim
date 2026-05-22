/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E Compare Match Timer device.
 *
 */

#ifndef DSH2ECMT_H_
#define DSH2ECMT_H_

#include "device.h"

extern device_type_t const dsh2ecmt;

#define PACKED __attribute__((packed))

#define device_get_sh2e_cmt(dev) ((sh2e_cmt_t *) (((peripheral_t *) (dev)->data)->data))

#define SH2E_CMT_CHANNELS_COUNT 2

// Link to hardware manual (REJ09B0045-0200H, page 445)

typedef union sh2e_cmt_cmstr {
    uint16_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint16_t _rf : 14; /** Reserved field, bits 15 to 2. */
        uint16_t str1 : 1; /** Selects whether to operate or halt compare match timer counter 1 */
        uint16_t str0 : 1; /** Selects whether to operate or halt compare match timer counter 0 */
#else
        uint16_t str0 : 1; /** Selects whether to operate or halt compare match timer counter 0 */
        uint16_t str1 : 1; /** Selects whether to operate or halt compare match timer counter 1 */
        uint16_t _rf : 14; /** Reserved field, bits 15 to 2. */
#endif
    };
} sh2e_cmt_cmstr_t;

typedef union sh2e_cmt_cmcsr {
    uint16_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint16_t _rf1 : 8; /** Reserved field 1, bits 15 to 8. These bits are always read as 0. The write value should always be 0. */
        uint16_t cmf : 1; /** Compare Match Flag - This flag indicates whether or not the CMCNT and CMCOR values have matched. */
        uint16_t cmie : 1; /** Compare Match Interrupt Enable - Selects whether to enable or disable a
                               compare match interrupt (CMI) when the CMCNT and CMCOR values have matched (CMF = 1)
                            */
        uint16_t _rf2 : 4; /** Reserved field 2, bits 5 to 2. */

        /**
         *  These bits select the clock input to
         *  CMCNT from among the four internal clocks obtained by dividing the peripheral clock (Pφ).
         *  When the STR bit of CMSTR is set to 1, CMCNT begins incrementing with the clock selected
         *  by CKS1 and CKS0.
         */
        uint16_t cks1 : 1; /** Clock Select 1  */
        uint16_t cks0 : 1; /** Clock Select 0  */
#else
        uint16_t cks0 : 1; /** Clock Select 0  */
        uint16_t cks1 : 1; /** Clock Select 1  */
        /**
         *  These bits select the clock input to
         *  CMCNT from among the four internal clocks obtained by dividing the peripheral clock (Pφ).
         *  When the STR bit of CMSTR is set to 1, CMCNT begins incrementing with the clock selected
         *  by CKS1 and CKS0.
         */

        uint16_t _rf2 : 4; /** Reserved field 2, bits 5 to 2. */
        uint16_t cmie : 1; /** Compare Match Interrupt Enable - Selects whether to enable or disable a
                       compare match interrupt (CMI) when the CMCNT and CMCOR values have matched (CMF = 1)
                    */
        uint16_t cmf : 1; /** Compare Match Flag - This flag indicates whether or not the CMCNT and CMCOR values have matched. */
        uint16_t _rf1 : 8; /** Reserved field 1, bits 15 to 8. These bits are always read as 0. The write value should always be 0. */

#endif
    };
} sh2e_cmt_cmcsr_t;

typedef struct sh2e_cmt_channel_reg {
    sh2e_cmt_cmcsr_t cmcsr; /* CMT control/status register */

    /**
     * CMT counter.
     * CMCNT is initialized to H'0000 by a power-on reset and in the standby modes.
     * It is not initialized by a manual reset.
     */
    uint16_t cmcnt;

    /**
     * CMT constant register.
     * CMCOR is initialized to H'FFFF by a power-on reset and in the standby modes.
     * It is not initialized by a manual reset.
     */
    uint16_t cmcor;
} sh2e_cmt_channel_reg_t;

typedef struct sh2e_cmt_regs {
    sh2e_cmt_cmstr_t cmstr; /* CMT start register */

    sh2e_cmt_channel_reg_t channels[SH2E_CMT_CHANNELS_COUNT]; /* CMT channel registers */
} sh2e_cmt_regs_t;

typedef struct sh2e_cmt {
    general_cpu_t *cpu; /* Pointer to the CPU this peripheral is attached to */

    uint64_t regs_addr; /* Base address of the CMT registers */

    sh2e_cmt_regs_t cmt_regs; /* CMT registers */

    unsigned int cpu_cycles; /* Number of CPU cycles since last tick */

    /* Internal counters */
    uint_fast16_t counter0;
    uint_fast16_t counter1;

    unsigned int int_no_first_channel; /* Interrupt number for the first channel */
    unsigned int int_no_second_channel; /* Interrupt number for the second channel */

    uint64_t interrupts_channel_0_count; /* Number of interrupts asserted from channel 0 */
    uint64_t interrupts_channel_1_count; /* Number of interrupts asserted from channel 1 */
} sh2e_cmt_t;

#endif
