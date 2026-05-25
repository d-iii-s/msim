/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#include <inttypes.h>
#include <stdio.h>

#include "../../../assert.h"
#include "../../../utils.h"
#include "debug.h"
#include "intc.h"

/** Processor register names in various styles. */

char const *const sh2e_intc_priority_reg_names[SH2E_INTC_IPR_REGISTERS_COUNT] = {
    "ipra",
    "iprb",
    "iprc",
    "iprd",
    "ipre",
    "iprf",
    "iprg",
    "iprh",
    "ipri",
    "iprj",
    "iprk",
    "iprl",
};

char const *const sh2e_intc_priority_reg_names_capitalized[SH2E_INTC_IPR_REGISTERS_COUNT] = {
    "IPRA",
    "IPRB",
    "IPRC",
    "IPRD",
    "IPRE",
    "IPRF",
    "IPRG",
    "IPRH",
    "IPRI",
    "IPRJ",
    "IPRK",
    "IPRL",
};

char const *const sh2e_intc_system_reg_names[SH2E_INTC_SYSTEM_REGISTERS_COUNT] = {
    "icr",
    "isr",
};

char const *const sh2e_intc_ipra_bit_indices[4] = {
    "15-12",
    "11-8",
    "7-4",
    "3-0",
};

typedef struct {
    const char *type;
    const char *priority_reg;
    const char *ipr_bits;
    bool pending;
} intc_row_t;

/**
 * @brief Dump the content of the INTC registers.
 */
void sh2e_intc_dump_regs(sh2e_intc_t const *const restrict intc)
{
    ASSERT(intc != NULL);

    printf("intc %u\n", intc->id);

    // General registers.
    for (unsigned int i = 0; i < SH2E_INTC_IPR_REGISTERS_COUNT; ++i) {
        printf(
                // I used this format in order to see all the digits as every digit
                // represents one priority pool index (4-bit value)
                " %5s: %04x\n",
                sh2e_intc_priority_reg_names[i],
                intc->intc_regs.priority[i]);
    }

    // System registers.
    for (unsigned int i = 0; i < SH2E_INTC_SYSTEM_REGISTERS_COUNT; ++i) {
        printf(
                " %5s: %4x\n",
                sh2e_intc_system_reg_names[i],
                intc->intc_regs.system[i]);
    }
}

void sh2e_intc_dump_configuration(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);

    printf("intc %u\n", intc->id);
    printf("[Interrupt no] [Type of interrupt      ] [Pending] [Priority register] [IPR bits]\n");

    for (unsigned int i = 0; i <= SH2E_INTC_SOURCE_MAX_VALUE; ++i) {
        if ((SH2E_INTC_VALID_SOURCE_ID(i) && intc->sources[i].registered) || (i == SH2E_INTC_NMI_VECTOR_ADDRESS_OFFSET)) {

            string_t int_no;
            string_t type;
            string_t pending;
            string_t priority_reg;
            string_t ipr_bits;

            string_init(&int_no);
            string_init(&type);
            string_init(&pending);
            string_init(&priority_reg);
            string_init(&ipr_bits);

            string_printf(&int_no, "%u", i);

            const char *type_str = "Internal interrupt";
            const char *prio_str = "-";
            const char *ipr_str = "-";
            bool pend = intc->sources[i].pending;

            switch (i) {

            case SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET: {
                type_str = "External power-on reset";
                break;
            }
            case SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET: {
                type_str = "Internal power-on reset";
                break;
            }

            case SH2E_INTC_MANUAL_RESET_OFFSET: {
                type_str = "Manual reset";
                break;
            }
            case SH2E_INTC_NMI_VECTOR_ADDRESS_OFFSET: {
                type_str = "NMI";
                pend = sh2e_check_nmi_interrupt(intc);
                break;
            }

            case SH2E_INTC_UBC_VECTOR_ADDRESS_OFFSET:
            case SH2E_INTC_HUDI_VECTOR_ADDRESS_OFFSET: {
                break;
            }

            case SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET ...(SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET + SH2E_INTC_IRQ_NUMBER_OF_SOURCES - 1): {
                unsigned int irq_id = i - SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET;
                static char irq_label[8];
                snprintf(irq_label, sizeof(irq_label), "IRQ%u", irq_id);
                type_str = irq_label;

                pend = (intc->intc_regs.isr.value >> (7 - i + SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET)) & 1;
                prio_str = sh2e_intc_priority_reg_names_capitalized[intc->sources[i].priority_pool_index / 4];
                ipr_str = sh2e_intc_ipra_bit_indices[intc->sources[i].priority_pool_index % 4];

                break;
            }

            default: {
                prio_str = sh2e_intc_priority_reg_names_capitalized[intc->sources[i].priority_pool_index / 4];
                ipr_str = sh2e_intc_ipra_bit_indices[intc->sources[i].priority_pool_index % 4];
                break;
            }
            }

            /* single formatting point */
            string_printf(&type, "%s", type_str);
            string_printf(&pending, "%s", pend ? "true" : "false");
            string_printf(&priority_reg, "%s", prio_str);
            string_printf(&ipr_bits, "%s", ipr_str);

            printf(" %12s ", int_no.str);
            printf("  %23s ", type.str);
            printf("  %7s ", pending.str);
            printf("  %17s ", priority_reg.str);
            printf("  %8s\n", ipr_bits.str);

            string_done(&int_no);
            string_done(&type);
            string_done(&pending);
            string_done(&priority_reg);
            string_done(&ipr_bits);
        }
    }
}
