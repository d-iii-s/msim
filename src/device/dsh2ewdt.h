/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E Watchdog Timer device.
 *
 */

#ifndef DSH2E_WDT_H_
#define DSH2E_WDT_H_

#include "cpu/general_cpu.h"
#include "device.h"

#define PACKED __attribute__((packed))
#define device_get_sh2e_wdt(dev) ((sh2e_wdt_t *) (((peripheral_t *) (dev)->data)->data))

extern device_type_t const dsh2ewdt;

// Link to hardware manual (REJ09B0045-0200H, page 431)

typedef union sh2e_wdt_tcsr {
    uint8_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint8_t ovf : 1; /** Bit 7 - Overflow flag. */
        uint8_t tms : 1; /** Bit 6 - Timer Mode Select. Selects whether to use the WDT as a watchdog timer or interval timer. */
        uint8_t tme : 1; /** Bit 5 - Timer Enable. Enables or disables the timer. */
        uint8_t _rf : 2; /** Reserved field, bits 4 to 3. */
        uint8_t cks2 : 1; /** Bits 2-0 - These bits select one of eight internal clock sources for input to TCNT.*/
        uint8_t cks1 : 1;
        uint8_t cks0 : 1;
#else
        uint8_t cks0 : 1;
        uint8_t cks1 : 1;
        uint8_t cks2 : 1; /** Bits 2-0 - These bits select one of eight internal clock sources for input to TCNT.*/
        uint8_t _rf : 2; /** Reserved field, bits 4 to 3. Must be 1 */
        uint8_t tme : 1; /** Bit 5 - Timer Enable. Enables or disables the timer. */
        uint8_t tms : 1; /** Bit 6 - Timer Mode Select. Selects whether to use the WDT as a watchdog timer or interval timer. */
        uint8_t ovf : 1; /** Bit 7 - Overflow flag. */
#endif
    };
} sh2e_wdt_tcsr_t;

typedef union sh2e_wdt_rstcsr {
    uint8_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint8_t wovf : 1; /** Bit 7 - Watchdog Timer Overflow Flag. */
        uint8_t rste : 1; /** Bit 6 - Reset Enable - Selects whether to reset the chip internally if TCNT overflows in watchdog timer mode. */
        uint8_t rsts : 1; /** Bit 5 - Reset Select - Selects the kind of internal reset to be generated when TCNT overflows in watchdog timer mode. */
        uint8_t _rf : 5; /** Reserved field, bits 4 to 0. */
#else
        uint8_t _rf : 5; /** Reserved field, bits 4 to 0. */
        uint8_t rsts : 1; /** Bit 5 - Reset Select - Selects the kind of internal reset to be generated when TCNT overflows in watchdog timer mode. */
        uint8_t rste : 1; /** Bit 6 - Reset Enable - Selects whether to reset the chip internally if TCNT overflows in watchdog timer mode. */
        uint8_t wovf : 1; /** Bit 7 - Watchdog Timer Overflow Flag. */
#endif
    };
} sh2e_wdt_rstcsr_t;

typedef struct sh2e_wdt_regs {
    sh2e_wdt_tcsr_t tcsr; /* WDT timer control/status register */

    uint8_t tcnt; /* WDT timer counter */

    sh2e_wdt_rstcsr_t rstcsr; /* WDT reset control/status register */

} sh2e_wdt_regs_t;

typedef struct sh2e_wdt {
    general_cpu_t *cpu; /* Pointer to the CPU that the WDT is attached to */

    uint64_t regs_addr; /* Base address of the WDT registers */

    sh2e_wdt_regs_t wdt_regs; /* WDT registers */

    unsigned int cpu_cycles; /* Number of CPU cycles since last tick */

    /* Internal counter */
    uint_fast16_t counter;

    unsigned int int_no; /* Interrupt number */

    uint64_t int_count; /* Number of interrupts asserted */
    uint64_t manual_resets_count; /* Number of manual resets */
    uint64_t power_on_resets_count; /* Number of power-on resets */
} sh2e_wdt_t;

#endif
