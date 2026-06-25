
#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <stddef.h>
#include <stdint.h>

#include "../list.h"

typedef void (*interrupt_func_t)(void *peripheral, unsigned int int_no);
typedef void (*update_cycles_func_t)(void *peripheral, unsigned int cycles);

typedef struct {
    interrupt_func_t interrupt_up_from_cpu; /** Signal an interrupt from CPU to the peripheral */
    interrupt_func_t interrupt_up_from_peripheral; /** Signal an interrupt from the peripheral to another peripheral */
    update_cycles_func_t update_cycles; /** Function which notifies the peripheral about cycle updates */
} peripheral_ops_t;

/** Structure describing a peripheral device */
typedef struct peripheral {
    const peripheral_ops_t *type;
    void *data;
} peripheral_t;

/** Structure describing a link to a peripheral device */
typedef struct peripheral_link {
    item_t item;
    peripheral_t *peripheral;
} peripheral_link_t;

#endif
