/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * General interface for interrupt controller devices.
 *
 */

#ifndef GENERAL_INTC_H_
#define GENERAL_INTC_H_

#include <stdbool.h>
#include <stdint.h>

#include "../../list.h"

#define MAX_INTCS 32

#define get_generic_intc(dev) (((general_intc_t *) (dev)->data))

/** Function type for raising and canceling interrupts */
typedef void (*interrupt_func_t)(void *, unsigned int);
/** Function type for checking pending interrupts */
typedef bool (*check_interrupts_func_t)(void *, uint8_t, uint8_t *);
/** Function type for accepting interrupt */
typedef void (*accept_interrupt_func_t)(void *, uint32_t *);
/** Function type for initializing the INTC */
typedef void (*init_intc_func_t)(void *);
/** Function type for checking pending resets */
typedef bool (*check_resets_func_t)(void *, uint8_t *);
/** Function type for accepting reset */
typedef void (*accept_reset_func_t)(void *);
typedef struct {
    interrupt_func_t interrupt_up; /** Raise an interrupt */
    interrupt_func_t interrupt_down; /** Cancel an interrupt */

    check_interrupts_func_t check_interrupts; /** Check for pending interrupts */
    accept_interrupt_func_t accept_interrupt; /** Accept an interrupt */

    check_resets_func_t check_resets; /** Check for pending resets */
    accept_reset_func_t accept_reset; /** Accept a reset */

    init_intc_func_t init; /** Initialize the INTC */
} intc_ops_t;

/** Structure describing INTC methods */
typedef struct {
    item_t item;
    unsigned int intcno;
    intc_ops_t const *type;
    void *data;
} general_intc_t;

/**
 * @brief Retrieves the general_intc_t structure based on the given cpu id
 */
extern general_intc_t *get_intc(unsigned int no);

/**
 * @brief Returns the lowest unused intc id or MAX_INTCS if none are available
 */
extern unsigned int get_free_intcno(void);

/**
 * @brief Adds the INTC to the list of all interrupt controllers
 */
extern void add_intc(general_intc_t *intc);

/**
 * @brief Removes the INTC from the list of all interrupt controllers
 */
extern void remove_intc(general_intc_t *intc);

/**
 * @brief Raises an interrupt
 *
 * @param intc The INTC on which the interrupt will be raised
 * @param no The interrupt number that will be raised
 */
extern void intc_interrupt_up(general_intc_t *intc, unsigned int no);
/**
 * @brief Cancels an interrupt
 *
 * @param intc The INTC on which the interrupt will be canceled
 * @param no The interrupt number, that will be canceled
 */
extern void intc_interrupt_down(general_intc_t *intc, unsigned int no);

extern bool intc_check_interrupts(general_intc_t *intc, uint8_t mask, uint8_t *interrupt_out);

extern void intc_accept_interrupt(general_intc_t *intc, uint32_t *new_mask_out);

extern bool intc_check_resets(general_intc_t *intc, uint8_t *reset_out);

extern void intc_accept_reset(general_intc_t *intc);

extern void intc_init(general_intc_t *intc);

#endif // GENERAL_INTC_H_
