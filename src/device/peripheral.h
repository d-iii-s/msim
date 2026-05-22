
#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <stddef.h>
#include <stdint.h>

#include "../list.h"

typedef void (*register_cpu_func_t)(void *peripheral, void *cpu);
typedef void (*interrupt_func_t)(void *peripheral, unsigned int int_no);
typedef void (*update_cycles_func_t)(void *peripheral, unsigned int cycles);

typedef struct {
    register_cpu_func_t register_cpu; /** Function which sends CPU pointer to the peripheral */
    interrupt_func_t interrupt_up; /** Signal an interrupt from CPU to the peripheral */
    update_cycles_func_t update_cycles; /** Function which notifies the peripheral about cycle updates */
} peripheral_ops_t;

/** Structure describing a peripheral device */
typedef struct {
    item_t item;
    const peripheral_ops_t *type;
    void *data;
} peripheral_t;

#endif
