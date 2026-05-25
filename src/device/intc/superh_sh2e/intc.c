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

#include <string.h>

#include "../../../arch/endianness.h"
#include "../../../assert.h"
#include "../../../physmem.h"
#include "intc.h"

#define address(ptr) (uintptr_t) (ptr)

sh2e_intc_icr_t
sh2e_intc_icr_reg_read(sh2e_intc_t *intc)
{
    return intc->intc_regs.icr;
}

sh2e_intc_isr_t
sh2e_intc_isr_reg_read(sh2e_intc_t *intc)
{
    return intc->intc_regs.isr;
}

void sh2e_intc_icr_reg_write(sh2e_intc_t *intc, uint16_t value)
{
    intc->intc_regs.icr.value = value;
}

void sh2e_intc_isr_reg_write(sh2e_intc_t *intc, uint16_t value)
{
    intc->intc_regs.isr.value = value;
}

uint16_t
sh2e_intc_priority_register_read(sh2e_intc_t *intc, uint8_t index)
{
    ASSERT(index < SH2E_INTC_IPR_REGISTERS_COUNT && "wrong index for priority register");
    return intc->intc_regs.priority[index];
}

void sh2e_intc_priority_register_write(sh2e_intc_t *intc, uint8_t index, uint16_t value)
{
    ASSERT(index < SH2E_INTC_IPR_REGISTERS_COUNT && "wrong index for priority register");
    intc->intc_regs.priority[index] = value;
}

void sh2e_intc_init_regs(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);

    // TODO: if the NMI pin is high the value of ICR should be H'8000
    memset(&intc->intc_regs, 0, sizeof(sh2e_intc_regs_t));
}

void sh2e_intc_init(sh2e_intc_t *intc, unsigned int id, uint64_t regs_addr)
{
    memset(intc, 0, sizeof(sh2e_intc_t));

    intc->id = id;
    intc->accepted_interrupts = 0;
    intc->accepted_resets = 0;
    intc->regs_addr = regs_addr;

    sh2e_intc_init_regs(intc);
}

void sh2e_intc_done(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);
    // Nothing to do here yet
}

void sh2e_intc_add_interrupt_source(sh2e_intc_t *intc, uint8_t source_id, uint8_t priority_pool_index, uint8_t priority)
{
    ASSERT(intc != NULL);
    ASSERT(SH2E_INTC_VALID_SOURCE_ID(source_id) && "Interrupt source ID out of range");
    ASSERT(priority_pool_index < SH2E_INTC_IPR_HALF_BYTES_LENGTH);
    ASSERT(priority <= SH2E_INTC_PRIORITY_MAX_VALUE);

    if (intc->sources[source_id].registered) {
        ASSERT(false && "Interrupt source with this ID already registered");
        return;
    }

    intc->sources[source_id].priority_pool_index = priority_pool_index;
    intc->sources[source_id].pending = false;
    intc->sources[source_id].registered = true;

    // Do not set priority into the registers for reset sources
    if (!SH2E_INTC_VALID_RESET_ID(source_id)) {
        // Need this in order to reverse the order and shift correctly
        int shift = (3 - (priority_pool_index % 4)) * 4;

        // NOTE: tha last value in the configuration with the same priority_pool_index will overwrite the previous ones
        intc->intc_regs.priority[priority_pool_index / 4] &= ~(0xF << shift);
        intc->intc_regs.priority[priority_pool_index / 4] |= (priority & 0xF) << shift;
    }
}

bool sh2e_check_nmi_interrupt(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);

    sh2e_intc_icr_t icr = intc->intc_regs.icr;
    bool detected = false;
    if (icr.nmie) {
        // NMI interrupt is detected on the rising edge of the NMI pin
        detected = intc->nmi_prev_value == 0 && icr.nmil;
    } else {
        // NMI interrupt is detected on the falling edge of the NMI pin
        detected = intc->nmi_prev_value != 0 && !icr.nmil;
    }
    return detected;
}

static bool sh2e_check_nmi_and_update_nmi_prev_value(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);

    bool detected = sh2e_check_nmi_interrupt(intc);
    intc->nmi_prev_value = intc->intc_regs.icr.nmil;
    return detected;
}

static bool
sh2e_check_irq_interrupt(sh2e_intc_t *intc, unsigned int irq_index)
{
    ASSERT(intc != NULL);
    ASSERT(irq_index < SH2E_INTC_IRQ_NUMBER_OF_SOURCES && "IRQ index out of range");

    uint8_t shift = 7 - irq_index;

    return (intc->intc_regs.isr.value >> shift) & 1;
}

static void
sh2e_assert_irq_interrupt(sh2e_intc_t *intc, unsigned int irq_index)
{
    ASSERT(intc != NULL);
    ASSERT(irq_index < SH2E_INTC_IRQ_NUMBER_OF_SOURCES && "IRQ index out of range");

    uint8_t shift = 7 - irq_index;

    intc->intc_regs.isr.value |= (1 << shift);
}

static void
sh2e_clear_irq_interrupt(sh2e_intc_t *intc, unsigned int irq_index)
{
    ASSERT(intc != NULL);
    ASSERT(irq_index < SH2E_INTC_IRQ_NUMBER_OF_SOURCES && "IRQ index out of range");

    uint8_t shift = 7 - irq_index;
    uint8_t irq_sense = (intc->intc_regs.icr.value >> shift) & 1;

    // Edge detection
    if (irq_sense) {
        intc->intc_regs.isr.value &= ~(1 << shift);
    }

    // If the level detection is used, the interrupt request remains set until the IRQ pin is cleared
}

static uint8_t
sh2e_get_priority(sh2e_intc_t *intc, sh2e_intc_source_t *source)
{
    ASSERT(source != NULL);

    uint8_t reg_index = source->priority_pool_index / 4;
    uint8_t half_byte_index = (source->priority_pool_index % 4);

    // Need this in order to reverse the order and shift correctly
    int shift = (3 - half_byte_index) * 4;

    return (intc->intc_regs.priority[reg_index] >> shift) & 0x0F;
}

bool sh2e_check_pending_interrupts(sh2e_intc_t *intc, uint8_t mask, uint8_t *interrupt_out)
{
    ASSERT(intc != NULL);
    ASSERT(interrupt_out != NULL);

    // Check NMI
    if (sh2e_check_nmi_and_update_nmi_prev_value(intc)) {
        intc->interrupt_out = SH2E_INTC_NMI_VECTOR_ADDRESS_OFFSET;
        intc->priority_out = SH2E_INTC_PRIORITY_MAX_VALUE;
        return SH2E_INTC_NMI_VECTOR_ADDRESS_OFFSET;
    }

    uint8_t interrupt_source = 0;
    uint32_t interrupt_priority = 0;

    // Check if any other interrupt is pending.
    for (unsigned int i = SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET + 1; i < SH2E_INTC_SOURCE_MAX_VALUE; i++) {
        if (SH2E_INTC_VALID_SOURCE_ID(i) && intc->sources[i].registered) {
            bool interrupt_pending = SH2E_INTC_VALID_IRQ_SOURCE_ID(i) ? sh2e_check_irq_interrupt(intc, i - SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET) : intc->sources[i].pending;
            if (interrupt_pending) {
                uint32_t curr_priority = (uint32_t) sh2e_get_priority(intc, &intc->sources[i]);
                if ((curr_priority > interrupt_priority || !interrupt_source) && curr_priority > mask) {
                    interrupt_priority = curr_priority;
                    interrupt_source = i;
                }
            }
        }
    }

    intc->priority_out = interrupt_priority;
    intc->interrupt_out = interrupt_source;

    // If it's other than 0, return true
    if (interrupt_source) {
        *interrupt_out = interrupt_source;
        return true;
    }

    return false;
}

bool sh2e_check_pending_resets(sh2e_intc_t *intc, uint8_t *reset_out)
{
    ASSERT(intc != NULL);
    ASSERT(reset_out != NULL);

    // Check for power-on reset (external)
    if (intc->sources[SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET].registered && intc->sources[SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET].pending) {
        *reset_out = SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET;
        return true;
    }

    // Check for power-on reset (internal)
    if (intc->sources[SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET].registered && intc->sources[SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET].pending) {
        *reset_out = SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET;
        return true;
    }

    // Check for manual reset
    if (intc->sources[SH2E_INTC_MANUAL_RESET_OFFSET].registered && intc->sources[SH2E_INTC_MANUAL_RESET_OFFSET].pending) {
        *reset_out = SH2E_INTC_MANUAL_RESET_OFFSET;
        return true;
    }

    return false;
}

void sh2e_accept_interrupt(sh2e_intc_t *intc, uint32_t *new_mask_out)
{
    ASSERT(intc != NULL);

    uint32_t new_mask = intc->priority_out;

    intc->priority_out = 0;

    // Clear the interrupt request
    if (SH2E_INTC_VALID_IRQ_SOURCE_ID(intc->interrupt_out)) {
        sh2e_clear_irq_interrupt(intc, intc->interrupt_out - SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET);
    } else {
        intc->sources[intc->interrupt_out].pending = false;
        intc->interrupt_out = 0;
    }

    if (new_mask_out != NULL) {
        *new_mask_out = new_mask;
    }
    intc->accepted_interrupts++;
}

void sh2e_accept_reset(sh2e_intc_t *intc)
{
    ASSERT(intc != NULL);

    // Clear all reset requests
    intc->sources[SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET].pending = false;
    intc->sources[SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET].pending = false;
    intc->sources[SH2E_INTC_MANUAL_RESET_OFFSET].pending = false;

    intc->accepted_resets++;
}

/** Assert the specified interrupt. */
void sh2e_intc_assert_interrupt(sh2e_intc_t *intc, unsigned int num)
{
    ASSERT(intc != NULL);
    ASSERT(SH2E_INTC_VALID_SOURCE_ID(num) && "Interrupt number out of range");
    ASSERT(intc->sources[num].registered && "Interrupt source not registered!");

    if (SH2E_INTC_VALID_IRQ_SOURCE_ID(num)) {
        sh2e_assert_irq_interrupt(intc, num - SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET);
        return;
    }

    intc->sources[num].pending = true;
}

/** Deassert the specified interrupt */
void sh2e_intc_deassert_interrupt(sh2e_intc_t *intc, unsigned int num)
{
    ASSERT(intc != NULL);
    ASSERT(SH2E_INTC_VALID_SOURCE_ID(num) && "Interrupt number out of range");
    ASSERT(intc->sources[num].registered && "Interrupt source not registered!");

    if (SH2E_INTC_VALID_IRQ_SOURCE_ID(num)) {
        sh2e_clear_irq_interrupt(intc, num - SH2E_INTC_IRQ_VECTOR_ADDRESS_OFFSET);
        return;
    }

    intc->sources[num].pending = false;
}
