/*
 * Copyright (c) 2026 Lubomir Bulej
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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../arch/endianness.h"
#include "../assert.h"
#include "../fault.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "dsh2ewdt.h"
#include "intc/superh_sh2e/intc.h"
#include "peripheral.h"

#define SH2E_WDT_REGISTERS_COUNT 3
#define SH2E_WDT_REGISTERS_START_ADDRESS UINT32_C(0xFFFFEC10)

#define SH2E_WDT_TCSR_READ_ADDRESS_OFFSET 0x0
#define SH2E_WDT_TCNT_READ_ADDRESS_OFFSET 0x1
#define SH2E_WDT_RSTCSR_READ_ADDRESS_OFFSET 0x3

#define SH2E_WDT_TCSR_WRITE_ADDRESS_OFFSET 0x0
#define SH2E_WDT_RSTCSR_WRITE_ADDRESS_OFFSET 0x2
#define SH2E_WDT_UPPER_BYTE_WRITE_KEY_A5 0xA5
#define SH2E_WDT_UPPER_BYTE_WRITE_KEY_5A 0x5A

#define SH2E_WDT_PERIOD_INTERVAL_1 2 /* Pφ/2 */
#define SH2E_WDT_PERIOD_INTERVAL_2 64 /* Pφ/64 */
#define SH2E_WDT_PERIOD_INTERVAL_3 128 /* Pφ/128 */
#define SH2E_WDT_PERIOD_INTERVAL_4 256 /* Pφ/256 */
#define SH2E_WDT_PERIOD_INTERVAL_5 512 /* Pφ/512 */
#define SH2E_WDT_PERIOD_INTERVAL_6 1024 /* Pφ/1024 */
#define SH2E_WDT_PERIOD_INTERVAL_7 4096 /* Pφ/4096 */
#define SH2E_WDT_PERIOD_INTERVAL_8 8192 /* Pφ/8192 */

static void sh2e_wdt_tcsr_reg_write(sh2e_wdt_t *wdt, uint8_t value)
{
    sh2e_wdt_tcsr_t new_value = { .value = value };

    sh2e_wdt_tcsr_t *reg = &wdt->wdt_regs.tcsr;

    reg->cks0 = new_value.cks0;
    reg->cks1 = new_value.cks1;
    reg->cks2 = new_value.cks2;
    reg->tme = new_value.tme;
    reg->tms = new_value.tms;

    // Writing 0 clears OVF, writing 1 has no effect
    if (!(new_value.ovf)) {
        reg->ovf = 0;
    }
}

static void sh2e_wdt_tcsr_reset(sh2e_wdt_t *wdt)
{
    wdt->wdt_regs.tcsr.value = 0;
    wdt->wdt_regs.tcsr._rf = 0x3; // Reserved bits must be 1
}

static void sh2e_wdt_tcnt_reset(sh2e_wdt_t *wdt)
{
    wdt->wdt_regs.tcnt = 0;
}

static void sh2e_wdt_rstcsr_reset(sh2e_wdt_t *wdt)
{
    wdt->wdt_regs.rstcsr.value = 0;
    wdt->wdt_regs.rstcsr._rf = 0x1F; // Reserved bits must be 1
}

/**
 * @brief Updates the internal cycle counter of the specified CMT in the system with the given number of cycles
 * @param cmt The CMT instance to update
 * @param cycles The number of cycles to add to the counter
 */
static void sh2e_wdt_cpu_cycles_update(void *peripheral, unsigned int cycles)
{
    sh2e_wdt_t *wdt = (sh2e_wdt_t *) ((peripheral_t *) peripheral)->data;
    wdt->cpu_cycles = cycles;
}

static void sh2e_wdt_interrupt_up(void *peripheral, unsigned int int_no)
{
    sh2e_wdt_t *wdt = (sh2e_wdt_t *) ((peripheral_t *) peripheral)->data;

    switch (int_no) {
    case SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET:
    case SH2E_INTC_SLEEP_MODE_OFFSET: {
        sh2e_wdt_tcsr_reset(wdt);
        sh2e_wdt_tcnt_reset(wdt);
        sh2e_wdt_rstcsr_reset(wdt);
        break;
    }
    case SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET: {
        sh2e_wdt_tcsr_reset(wdt);
        sh2e_wdt_tcnt_reset(wdt);
        break;
    }
    }
}

static peripheral_ops_t const sh2e_wdt_peripheral_ops = {
    .interrupt_up = (interrupt_func_t) sh2e_wdt_interrupt_up,
    .update_cycles = (update_cycles_func_t) sh2e_wdt_cpu_cycles_update
};

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dsh2ewdt_init(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    parm_next(&parm);

    uint64_t int_no = parm_uint_next(&parm);

    uint64_t _addr;
    if (parm_type(parm) == tt_end) {
        _addr = SH2E_WDT_REGISTERS_START_ADDRESS;
    } else {
        _addr = parm_uint_next(&parm);
    }

    if (!phys_range(_addr)) {
        error("Physical memory address out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) (SH2E_WDT_REGISTERS_COUNT * sizeof(uint8_t)))) {
        error("Invalid address, registers would exceed the physical "
              "memory range");
        return false;
    }

    ptr36_t addr = _addr;

    // TODO: maybe not so strict on alignment?
    if (!ptr36_dword_aligned(addr)) {
        error("Physical memory address must be 8-byte aligned");
        return false;
    }

    /* Initialization */
    sh2e_wdt_t *sh2e_wdt = safe_malloc_t(sh2e_wdt_t);

    sh2e_wdt->regs_addr = addr;
    sh2e_wdt->int_no = int_no;

    sh2e_wdt->cpu_cycles = 0;
    sh2e_wdt->int_count = 0;

    sh2e_wdt_tcsr_reset(sh2e_wdt);
    sh2e_wdt_tcnt_reset(sh2e_wdt);
    sh2e_wdt_rstcsr_reset(sh2e_wdt);

    peripheral_t *generic_peripheral = safe_malloc_t(peripheral_t);
    *generic_peripheral = (peripheral_t) {
        .data = sh2e_wdt,
        .type = &sh2e_wdt_peripheral_ops,
    };

    item_init(&generic_peripheral->item);
    dev->data = generic_peripheral;

    return true;
}

/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dsh2ewdt_info(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    printf("SH-2E Watchdog Timer (WDT)\n");
    return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dsh2ewdt_stat(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);

    printf("[WDT interrupts] [WDT power-on resets] [WDT manual resets]\n");
    printf("%16" PRIu64 " %21" PRIu64 " %19" PRIu64 "\n\n", wdt->int_count, wdt->power_on_resets_count, wdt->manual_resets_count);

    return true;
}

/** Read byte command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Pointer to store the read value
 *
 */
static void dsh2ewdt_read8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t *val)
{
    ASSERT(dev != NULL);

    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);
    ptr36_t offset = addr - wdt->regs_addr;

    switch (offset) {
    case SH2E_WDT_TCSR_READ_ADDRESS_OFFSET: {
        *val = wdt->wdt_regs.tcsr.value;
        break;
    }
    case SH2E_WDT_TCNT_READ_ADDRESS_OFFSET: {
        *val = wdt->wdt_regs.tcnt;
        break;
    }
    case SH2E_WDT_RSTCSR_READ_ADDRESS_OFFSET: {
        *val = wdt->wdt_regs.rstcsr.value;
        break;
    }
    }
}

/** Read word command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Pointer to store the read value
 *
 */
static void dsh2ewdt_read16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t *val)
{
    ASSERT(dev != NULL);

    ptr36_t wdt_start = SH2E_WDT_REGISTERS_START_ADDRESS;
    ptr36_t wdt_end = wdt_start + (SH2E_WDT_REGISTERS_COUNT * sizeof(uint8_t));

    if (addr >= wdt_start && addr < wdt_end) {
        error("Reading from the registers by a word transfer function if not supported. Registers must be read by byte transfer functions.");
    }
}

/** Read longword command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Pointer to store the read value
 *
 * @return Read value
 *
 */
static void dsh2ewdt_read32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t *val)
{
    ASSERT(dev != NULL);

    ptr36_t wdt_start = SH2E_WDT_REGISTERS_START_ADDRESS;
    ptr36_t wdt_end = wdt_start + (SH2E_WDT_REGISTERS_COUNT * sizeof(uint8_t));

    if (addr >= wdt_start && addr < wdt_end) {
        error("Reading from the registers by a longword transfer function if not supported. Registers must be read by byte transfer functions.");
    }
}

/** Write byte command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2ewdt_write8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t val)
{
    ASSERT(dev != NULL);

    ptr36_t wdt_start = SH2E_WDT_REGISTERS_START_ADDRESS;
    ptr36_t wdt_end = wdt_start + (SH2E_WDT_REGISTERS_COUNT * sizeof(uint8_t));

    if (addr >= wdt_start && addr < wdt_end) {
        error("Writing to the registers by a byte transfer function if not supported. Registers must be written to by word transfer functions.");
    }
}

/** Write word command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2ewdt_write16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t val)
{
    ASSERT(dev != NULL);

    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);

    ptr36_t offset = addr - wdt->regs_addr;

    uint16_t be_value = be16toh(val);

    uint8_t upper = (be_value >> 8) & 0xFF;
    uint8_t lower = be_value & 0xFF;

    switch (offset) {
    case SH2E_WDT_TCSR_WRITE_ADDRESS_OFFSET: {
        switch (upper) {
        case SH2E_WDT_UPPER_BYTE_WRITE_KEY_A5: {
            sh2e_wdt_tcsr_reg_write(wdt, lower);
            break;
        }
        case SH2E_WDT_UPPER_BYTE_WRITE_KEY_5A: {
            wdt->wdt_regs.tcnt = lower;
            break;
        }
        default: {
            error("Invalid write key for WDT registers. Expected 0xA5 for TCSR or 0x5A for TCNT in the upper byte.");
            break;
        }
        }
        break;
    }
    case SH2E_WDT_RSTCSR_WRITE_ADDRESS_OFFSET: {
        sh2e_wdt_rstcsr_t new_value = { .value = lower };
        switch (upper) {
        case SH2E_WDT_UPPER_BYTE_WRITE_KEY_A5: {
            if (lower == 0) {
                wdt->wdt_regs.rstcsr.wovf = 0;
            } else {
                error("Invalid write to the WDT RSTCSR register. To clear the WOVF bit, write 0 to the lower byte with 0xA5 in the upper byte.");
            }
            break;
        }
        case SH2E_WDT_UPPER_BYTE_WRITE_KEY_5A: {
            wdt->wdt_regs.rstcsr.rste = new_value.rste;
            wdt->wdt_regs.rstcsr.rsts = new_value.rsts;
            break;
        }
        default: {
            error("Invalid write key for RSTCSR WDT. Expected 0xA5 or 0x5A in the upper byte.");
            break;
        }
        }
        break;
    }
    }
}

/** Write longword command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Pointer to store the read value
 *
 * @return Read value
 *
 */
static void dsh2ewdt_write32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t val)
{
    ASSERT(dev != NULL);
    ptr36_t wdt_start = SH2E_WDT_REGISTERS_START_ADDRESS;
    ptr36_t wdt_end = wdt_start + (SH2E_WDT_REGISTERS_COUNT * sizeof(uint8_t));

    if (addr >= wdt_start && addr < wdt_end) {
        error("Writing to the registers by a longword transfer function if not supported. Registers must be written to by word transfer functions.");
    }
}

/** WDT step implementation
 *
 * @param dev Device pointer
 *
 */
static void dsh2ewdt_step(device_t *dev)
{
    ASSERT(dev != NULL);

    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);

    // Check if the timer is enabled
    if (wdt->wdt_regs.tcsr.tme) {
        uint16_t period;

        switch ((wdt->wdt_regs.tcsr.cks2 << 2) | (wdt->wdt_regs.tcsr.cks1 << 1) | wdt->wdt_regs.tcsr.cks0) {
        case 0: {
            period = SH2E_WDT_PERIOD_INTERVAL_1;
            break;
        }
        case 1: {
            period = SH2E_WDT_PERIOD_INTERVAL_2;
            break;
        }
        case 2: {
            period = SH2E_WDT_PERIOD_INTERVAL_3;
            break;
        }
        case 3: {
            period = SH2E_WDT_PERIOD_INTERVAL_4;
            break;
        }
        case 4: {
            period = SH2E_WDT_PERIOD_INTERVAL_5;
            break;
        }
        case 5: {
            period = SH2E_WDT_PERIOD_INTERVAL_6;
            break;
        }
        case 6: {
            period = SH2E_WDT_PERIOD_INTERVAL_7;
            break;
        }
        case 7: {
            period = SH2E_WDT_PERIOD_INTERVAL_8;
            break;
        }
        }

        wdt->counter += wdt->cpu_cycles;

        while (wdt->counter >= period) {
            wdt->counter -= period;

            wdt->wdt_regs.tcnt++;
            if (wdt->wdt_regs.tcsr.tms) {
                // Watchdog timer mode
                if (wdt->wdt_regs.tcnt == 0) {
                    // Overflow
                    wdt->wdt_regs.rstcsr.wovf = 1;

                    if (wdt->wdt_regs.rstcsr.rste) {
                        if (wdt->wdt_regs.rstcsr.rsts) {
                            // Manual reset
                            // TODO: maybe add a constant to the device configuration and use that instead of hardcoding this to the SH-2E INTC
                            cpu_interrupt_up(wdt->cpu, SH2E_INTC_MANUAL_RESET_OFFSET);
                            wdt->manual_resets_count++;
                        } else {
                            // Power-on reset
                            cpu_interrupt_up(wdt->cpu, SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET);
                            wdt->power_on_resets_count++;
                        }
                    } else {
                        sh2e_wdt_tcsr_reset(wdt);
                        sh2e_wdt_tcnt_reset(wdt);
                    }
                }
            } else {
                // Interval timer mode
                if (wdt->wdt_regs.tcnt == 0) {
                    // Overflow
                    wdt->wdt_regs.tcsr.ovf = 1;

                    // Assert interrupt
                    cpu_interrupt_up(wdt->cpu, wdt->int_no);
                    wdt->int_count++;
                }
            }
        }
    }

    wdt->cpu_cycles = 0;
}

/** Dump WDT registers command. */
static bool
dsh2ewdt_cmd_dump_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);

    printf("WDT\n");

    printf(" %6s: %02x\n", "tcsr", wdt->wdt_regs.tcsr.value);
    printf(" %6s: %02x\n", "tcnt", wdt->wdt_regs.tcnt);
    printf(" %6s: %02x\n", "rstcsr", wdt->wdt_regs.rstcsr.value);

    return true;
}

static bool dsh2e_wdt_add_cpu(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    const char *cpu_name = parm_str_next(&parm);
    device_t *cpu_dev = dev_by_name(cpu_name);

    if (cpu_dev == NULL) {
        error("CPU device '%s' not found", cpu_name);
        return false;
    }

    general_cpu_t *cpu = cpu_dev->data;
    sh2e_wdt_t *wdt = device_get_sh2e_wdt(dev);

    wdt->cpu = cpu;

    return true;
}

/** Dispose dsh2ewdt
 *
 * @param dev Device pointer
 *
 */
static void dsh2ewdt_done(device_t *dev)
{
    ASSERT(dev != NULL);

    safe_free(dev->data);
}

static cmd_t dsh2ewdt_cmds[] = {
    { "init",
            (fcmd_t) dsh2ewdt_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/timer name" NEXT
                    REQ INT "interrupt number" NEXT
                            OPT INT "addr/register block address" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display help",
            "Display help",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dsh2ewdt_info,
            DEFAULT,
            DEFAULT,
            "Display device configuration",
            "Display device configuration",
            NOCMD },
    { "stat",
            (fcmd_t) dsh2ewdt_stat,
            DEFAULT,
            DEFAULT,
            "Display device statictics",
            "display device statictics",
            NOCMD },
    { "rd",
            (fcmd_t) dsh2ewdt_cmd_dump_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of WDT registers",
            "Dump contents of WDT registers",
            NOCMD },
    { "addcpu",
            (fcmd_t) dsh2e_wdt_add_cpu,
            DEFAULT,
            DEFAULT,
            "Add CPU to the device",
            "Add CPU to the device",
            REQ STR "cpu name" END },
    LAST_CMD
};

device_type_t const dsh2ewdt = {
    .nondet = false,

    /* Type name and description */
    .name = "dsh2ewdt",
    .brief = "Watchdog Timer (WDT) device for SH-2E",
    .full = "This device simulates the on-chip Watchdog Timer (WDT) of the SH-2E microprocessor.",

    /* Functions */
    .read8 = dsh2ewdt_read8,
    .read16 = dsh2ewdt_read16,
    .read32 = dsh2ewdt_read32,

    .write8 = dsh2ewdt_write8,
    .write16 = dsh2ewdt_write16,
    .write32 = dsh2ewdt_write32,

    .step = dsh2ewdt_step,
    .done = dsh2ewdt_done,

    /* Commands */
    .cmds = dsh2ewdt_cmds
};
