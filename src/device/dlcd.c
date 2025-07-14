/*
 * Copyright (c) 2002-2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  HD44780U LCD module device
 *
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../assert.h"
#include "../fault.h"
#include "../parser.h"
#include "../text.h"
#include "../utils.h"
#include "device.h"
#include "dlcd.h"

#define REGISTER_DATA 0 /**< Offset of data/command register */
#define REGISTER_CONTROL 1 /**< Offset of control register */
#define REGISTER_LIMIT 4 /**< Size of the register block */

#define LCD_MAX_DDRAM_SIZE 80 /* Maximum DDRAM characters supported by HD44780 */
#define LCD_MAX_ROWS 4 /* Maximum rows supported */
#define LCD_MAX_COLS 40 /* Maximum columns per row supported */

static uint8_t row_addr_map[LCD_MAX_ROWS] = {
    0x00, 0x40, 0x14, 0x54
}; /**< Row address map for HD44780 */

typedef union {
    uint32_t value;
    struct {
        bool rs : 1; /**< Register select */
        bool rw : 1; /**< Read/Write */
        bool e : 1; /**< Enable */
        int _ : 5;

        uint8_t data : 8; /**< Data/command register */
    } parts;
} lcd_reg_t;

typedef struct {
    int rows;
    int cols;

    int current_row;
    int current_col;

    lcd_reg_t reg; /**< Register value */
    lcd_reg_t reg_prev; /**< Previous register value */

    uint8_t *buffer;

    uint64_t addr; /**< Register address */
} lcd_data_t;

static bool dlcd_init(token_t *parm, device_t *dev)
{
    parm_next(&parm);
    uint cols = parm_uint(parm);

    if (cols > LCD_MAX_COLS) {
        error("Number of columns exceeds maximum (%d)", LCD_MAX_COLS);
        return false;
    }

    parm_next(&parm);
    uint rows = parm_uint(parm);

    if (rows > LCD_MAX_ROWS) {
        error("Number of rows exceeds maximum (%d)", LCD_MAX_ROWS);
        return false;
    }

    parm_next(&parm);
    uint64_t _addr = parm_uint(parm);

    if (!phys_range(_addr)) {
        error("Physical memory address of data register is out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) REGISTER_LIMIT)) {
        error("Invalid address, registers would exceed the physical "
              "memory range");
        return false;
    }

    uint64_t addr = _addr;

    lcd_data_t *data = safe_malloc_t(lcd_data_t);
    dev->data = data;

    data->rows = rows;
    data->cols = cols;
    data->current_row = 0;
    data->current_col = 0;

    data->buffer = safe_malloc(sizeof(uint8_t) * rows * cols);
    memset(data->buffer, 0, rows * cols);

    data->addr = addr;

    return true;
}

static void lcd_done(device_t *dev)
{
    lcd_data_t *data = (lcd_data_t *) dev->data;

    safe_free(data->buffer);
    safe_free(data);
}

static void lcd_print(lcd_data_t *data)
{
    printf("┌");

    for (int i = 0; i < data->cols; i++) {
        printf("─");
    }

    printf("┐\n");

    for (int row = 0; row < data->rows; row++) {
        printf("│");

        for (int col = 0; col < data->cols; col++) {
            char c = data->buffer[row * data->cols + col];

            printf("%c", (c == 0) ? ' ' : c);
        }

        printf("│\n");
    }

    printf("└");

    for (int i = 0; i < data->cols; i++) {
        printf("─");
    }

    printf("┘\n");

    fflush(stdout);
}

static bool ddram_addr_to_position(lcd_data_t *data, uint8_t addr, int *row, int *col)
{
    for (int i = 0; i < data->rows; i++) {
        uint8_t base = row_addr_map[i];

        if (addr >= base && addr < base + LCD_MAX_COLS) {
            *row = i;
            *col = addr - base;

            // ensure column is within bounds
            if (*col >= data->cols) {
                *col = data->cols - 1;
            }

            return true;
        }
    }

    return false;
}

static void lcd_set_cursor(lcd_data_t *data, uint8_t addr)
{
    int row, col;

    if (ddram_addr_to_position(data, addr, &row, &col)) {
        data->current_row = row;
        data->current_col = col;
    }
}

static void lcd_write32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t val)
{
    ASSERT(dev != NULL);

    lcd_data_t *data = (lcd_data_t *) dev->data;
    bool display_updated = false;

    switch (addr - data->addr) {
    case REGISTER_DATA:
        data->reg.parts.data = (uint8_t) val;
        break;

    case REGISTER_CONTROL:
        data->reg_prev.value = data->reg.value;

        lcd_reg_t value = { .value = val };

        data->reg.parts.rs = value.parts.rs;
        data->reg.parts.rw = value.parts.rw;
        data->reg.parts.e = value.parts.e;

        // action should be only taken on a falling edge of the enable bit
        if ((data->reg_prev.parts.e) && !(data->reg.parts.e)) {
            if (data->reg.parts.rs) {
                // RS = 1 => writing to data register
                if (!data->reg.parts.rw) {
                    // RW = 0 => write operation

                    if (data->current_col < data->cols) {
                        data->buffer[data->current_row * data->cols + data->current_col] = data->reg.parts.data;
                        data->current_col++;
                        display_updated = true;

                        // automatically wrap to next line if needed
                        if (data->current_col >= data->cols) {
                            data->current_col = 0;
                            data->current_row = (data->current_row + 1) % data->rows;
                        }
                    }
                }
            } else {
                // RS = 0 => writing to command register
                if (!data->reg.parts.rw) {
                    // RW = 0 => write operation
                    switch (data->reg.parts.data) {
                    case 0x01: /* clear display */
                        memset(data->buffer, 0, data->rows * data->cols);

                        data->current_row = 0;
                        data->current_col = 0;
                        display_updated = true;

                        break;

                    case 0x02: /* return home */
                        data->current_row = 0;
                        data->current_col = 0;

                        break;

                    default:
                        if ((data->reg.parts.data & 0x80) == 0x80) {
                            uint8_t addr = data->reg.parts.data & 0x7F;

                            lcd_set_cursor(data, addr);
                        }

                        break;
                    }
                }
            }

            if (display_updated) {
                lcd_print(data);
            }
        }
        break;

    default:
        break;
    }
}

static bool dlcd_info(token_t *parm, device_t *dev)
{
    lcd_data_t *data = (lcd_data_t *) dev->data;

    printf("[data register]\n");
    printf("%#11" PRIx64 "\n", data->addr);
    printf("[control register]\n");
    printf("%#11" PRIx64 "\n", data->addr + 1);

    return true;
}

static cmd_t lcd_cmds[] = {
    { "init",
            (fcmd_t) dlcd_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/lcd name" NEXT
                    REQ INT "rows/number of rows" NEXT
                            REQ INT "columns/number of columns" NEXT
                                    REQ INT "register/address of the register" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display this help text",
            "Display this help text",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dlcd_info,
            DEFAULT,
            DEFAULT,
            "Display LCD state and configuration",
            "Display LCD state and configuration",
            NOCMD },
    LAST_CMD
};

device_type_t dlcd = {
    /* LCD is a deterministic device */
    .nondet = false,

    .name = "dlcd",
    .brief = "LCD and shift register module simulation",
    .full = "LCD and shift register module simulation",

    .done = lcd_done,
    .write32 = lcd_write32,

    .cmds = lcd_cmds
};
