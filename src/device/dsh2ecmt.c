/*
 * Copyright (c) 2026 Lubomir Bulej
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
#include "dsh2ecmt.h"
#include "intc/superh_sh2e/intc.h"
#include "peripheral.h"

#define SH2E_CMT_CHANNEL_REGISTERS_COUNT 3
#define SH2E_CMT_REGISTERS_START_ADDRESS UINT32_C(0xFFFFF710)
#define SH2E_CMT_CMCOR_INITIAL_VALUE UINT16_C(0xFFFF)
#define SH2E_CMT_CMSTR_MASK 0x3

#define SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET 0x0
#define SH2E_CMT_CMCSR0_REGISTER_ADDRESS_OFFSET 0x2
#define SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET 0x4
#define SH2E_CMT_CMCOR0_REGISTER_ADDRESS_OFFSET 0x6
#define SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET 0x8
#define SH2E_CMT_CMCNT1_REGISTER_ADDRESS_OFFSET 0xA
#define SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET 0xC

#define SH2E_CMT_PERIOD_INTERVAL_1 8 /* Pφ/8 */
#define SH2E_CMT_PERIOD_INTERVAL_2 32 /* Pφ/32 */
#define SH2E_CMT_PERIOD_INTERVAL_3 128 /* Pφ/128 */
#define SH2E_CMT_PERIOD_INTERVAL_4 512 /* Pφ/512 */

#define SH2E_CMT_CMSTR_NAME "cmstr"

char const *const sh2e_cmt_channel_reg_names[SH2E_CMT_CHANNEL_REGISTERS_COUNT] = {
    "cmcsr",
    "cmcnt",
    "cmcor"
};

static void sh2e_cmt_cmstr_reg_write(sh2e_cmt_t *cmt, uint16_t value)
{
    cmt->cmt_regs.cmstr.value = value & SH2E_CMT_CMSTR_MASK;
}

static void sh2e_cmt_cmcsr_reg_write(sh2e_cmt_channel_reg_t *channel_reg, uint16_t value)
{
    sh2e_cmt_cmcsr_t new_value = { .value = value };

    sh2e_cmt_cmcsr_t *reg = &channel_reg->cmcsr;

    reg->cks0 = new_value.cks0;
    reg->cks1 = new_value.cks1;
    reg->cmie = new_value.cmie;

    // Writing 0 clears CMF, writing 1 has no effect
    if (!(new_value.cmf)) {
        reg->cmf = 0;
    }
}

static void read_byte_from_register(uint16_t *reg, bool upper_byte, uint8_t *output_value)
{
    *output_value = upper_byte ? (*reg >> 8) & 0xFF : *reg & 0xFF;
}

static void write_byte_to_register(uint16_t *reg, bool upper_byte, uint8_t val)
{
    if (upper_byte) {
        *reg = ((*reg) & 0x00FF) | ((uint16_t) val << 8);
    } else {
        *reg = ((*reg) & 0xFF00) | val;
    }
}

/**
 * @brief Resets the CMT device
 */
static void sh2e_cmt_reset(sh2e_cmt_t *cmt)
{
    // Also initialize the counters to 0
    cmt->counter0 = 0;
    cmt->counter1 = 0;

    memset(&(cmt->cmt_regs), 0, sizeof(sh2e_cmt_regs_t));

    // The constant registers are initialized to 0xFFFF
    for (unsigned int i = 0; i < SH2E_CMT_CHANNELS_COUNT; i++) {
        cmt->cmt_regs.channels[i].cmcor = SH2E_CMT_CMCOR_INITIAL_VALUE;
    }
}

/**
 * @brief Updates the internal cycle counter of the specified CMT in the system with the given number of cycles
 * @param cmt The CMT instance to update
 * @param cycles The number of cycles to add to the counter
 */
static void sh2e_cmt_cpu_cycles_update(void *peripheral, unsigned int cycles)
{
    sh2e_cmt_t *cmt = (sh2e_cmt_t *) ((peripheral_t *) peripheral)->data;
    cmt->cpu_cycles = cycles;
}

static void sh2e_cmt_interrupt_up(void *peripheral, unsigned int int_no)
{
    sh2e_cmt_t *cmt = (sh2e_cmt_t *) ((peripheral_t *) peripheral)->data;

    switch (int_no) {
    case SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET:
    case SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET:
    case SH2E_INTC_SLEEP_MODE_OFFSET: {
        sh2e_cmt_reset(cmt);
        break;
    }
    }
}

static peripheral_ops_t const sh2e_cmt_peripheral_ops = {
    .interrupt_up_from_cpu = (interrupt_func_t) sh2e_cmt_interrupt_up,
    .update_cycles = (update_cycles_func_t) sh2e_cmt_cpu_cycles_update
};

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dsh2ecmt_init(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    parm_next(&parm);

    uint64_t int_no_first_channel = parm_uint_next(&parm);
    uint64_t int_no_second_channel = parm_uint_next(&parm);

    uint64_t _addr;
    if (parm_type(parm) == tt_end) {
        _addr = SH2E_CMT_REGISTERS_START_ADDRESS;
    } else {
        _addr = parm_uint_next(&parm);
    }

    if (!phys_range(_addr)) {
        error("Physical memory address out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) ((SH2E_CMT_CHANNELS_COUNT * sizeof(sh2e_cmt_channel_reg_t)) + sizeof(uint16_t)))) {
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
    sh2e_cmt_t *sh2e_cmt = safe_malloc_t(sh2e_cmt_t);

    sh2e_cmt->regs_addr = addr;
    sh2e_cmt->int_no_first_channel = int_no_first_channel;
    sh2e_cmt->int_no_second_channel = int_no_second_channel;

    sh2e_cmt->cpu_cycles = 0;
    sh2e_cmt->interrupts_channel_0_count = 0;
    sh2e_cmt->interrupts_channel_1_count = 0;

    sh2e_cmt_reset(sh2e_cmt);

    peripheral_t *generic_peripheral = safe_malloc_t(peripheral_t);
    *generic_peripheral = (peripheral_t) {
        .data = sh2e_cmt,
        .type = &sh2e_cmt_peripheral_ops,
    };

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
static bool dsh2ecmt_info(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    printf("SH-2E Compare Match Timer (CMT)\n");
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
static bool dsh2ecmt_stat(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *cmt = device_get_sh2e_cmt(dev);

    printf("[CMT0 interrupts] [CMT1 interrupts] [Total interrupts]\n");
    printf("%17" PRIu64 " %17" PRIu64 " %18" PRIu64 "\n\n",
            cmt->interrupts_channel_0_count,
            cmt->interrupts_channel_1_count,
            cmt->interrupts_channel_0_count + cmt->interrupts_channel_1_count);

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
static void dsh2ecmt_read8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t *val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);
    ptr36_t offset = addr - sh2e_cmt->regs_addr;

    bool upper_byte = (offset % 2) == 0;
    unsigned int channel_num = (offset >= SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET) ? 1 : 0;

    offset -= offset % 2; // Align to 16-bit register

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        read_byte_from_register(&sh2e_cmt->cmt_regs.cmstr.value, upper_byte, val);
        break;
    }
    case SH2E_CMT_CMCSR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        read_byte_from_register(&sh2e_cmt->cmt_regs.channels[channel_num].cmcsr.value, upper_byte, val);
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCNT1_REGISTER_ADDRESS_OFFSET: {
        read_byte_from_register(&sh2e_cmt->cmt_regs.channels[channel_num].cmcnt, upper_byte, val);
        break;
    };
    case SH2E_CMT_CMCOR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        read_byte_from_register(&sh2e_cmt->cmt_regs.channels[channel_num].cmcor, upper_byte, val);
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
static void dsh2ecmt_read16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t *val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    ptr36_t offset = addr - sh2e_cmt->regs_addr;
    unsigned int channel_num = (offset >= SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET) ? 1 : 0;

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        *val = sh2e_cmt->cmt_regs.cmstr.value;
        break;
    }
    case SH2E_CMT_CMCSR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        *val = sh2e_cmt->cmt_regs.channels[channel_num].cmcsr.value;
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCNT1_REGISTER_ADDRESS_OFFSET: {
        *val = sh2e_cmt->cmt_regs.channels[channel_num].cmcnt;
        break;
    }
    case SH2E_CMT_CMCOR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        *val = sh2e_cmt->cmt_regs.channels[channel_num].cmcor;
        break;
    }
    }

    *val = htobe16(*val);
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
static void dsh2ecmt_read32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t *val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    ptr36_t offset = addr - sh2e_cmt->regs_addr;

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        *val = (uint32_t) (sh2e_cmt->cmt_regs.cmstr.value << 16) | (uint32_t) sh2e_cmt->cmt_regs.channels[0].cmcsr.value;
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET: {
        *val = (uint32_t) (sh2e_cmt->cmt_regs.channels[0].cmcnt << 16) | (uint32_t) sh2e_cmt->cmt_regs.channels[0].cmcor;
        break;
    }
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        *val = (uint32_t) (sh2e_cmt->cmt_regs.channels[1].cmcsr.value << 16) | (uint32_t) sh2e_cmt->cmt_regs.channels[1].cmcnt;
        break;
    }
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        // This case is a bit special, as there is no register that would be read together with CMCOR1.
        // We will return the value of CMCOR1 in the upper half of the returned value, and 0 in the lower half.
        *val = (sh2e_cmt->cmt_regs.channels[1].cmcor << 16);
        break;
    }
    }

    *val = htobe32(*val);
}

/** Write byte command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2ecmt_write8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    ptr36_t offset = addr - sh2e_cmt->regs_addr;
    bool upper_byte = (offset % 2) == 0;
    unsigned int channel_num = (offset >= SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET) ? 1 : 0;

    offset -= offset % 2; // Align to 16-bit register

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        uint16_t tmp_cmstr = sh2e_cmt->cmt_regs.cmstr.value;
        write_byte_to_register(&tmp_cmstr, upper_byte, val);
        sh2e_cmt_cmstr_reg_write(sh2e_cmt, tmp_cmstr);
        break;
    }
    case SH2E_CMT_CMCSR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        uint16_t tmp_cmcsr = sh2e_cmt->cmt_regs.channels[channel_num].cmcsr.value;
        write_byte_to_register(&tmp_cmcsr, upper_byte, val);
        sh2e_cmt_cmcsr_reg_write(&sh2e_cmt->cmt_regs.channels[channel_num], tmp_cmcsr);
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCNT1_REGISTER_ADDRESS_OFFSET: {
        write_byte_to_register(&sh2e_cmt->cmt_regs.channels[channel_num].cmcnt, upper_byte, val);
        break;
    }
    case SH2E_CMT_CMCOR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        write_byte_to_register(&sh2e_cmt->cmt_regs.channels[channel_num].cmcor, upper_byte, val);
        break;
    }
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
static void dsh2ecmt_write16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    ptr36_t offset = addr - sh2e_cmt->regs_addr;
    unsigned int channel_num = (offset >= SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET) ? 1 : 0;

    uint16_t be_value = be16toh(val);

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt_cmstr_reg_write(sh2e_cmt, be_value);
        break;
    }
    case SH2E_CMT_CMCSR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt_cmcsr_reg_write(&sh2e_cmt->cmt_regs.channels[channel_num], be_value);
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCNT1_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt->cmt_regs.channels[channel_num].cmcnt = be_value;
        break;
    }
    case SH2E_CMT_CMCOR0_REGISTER_ADDRESS_OFFSET:
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt->cmt_regs.channels[channel_num].cmcor = be_value;
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
static void dsh2ecmt_write32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t val)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    ptr36_t offset = addr - sh2e_cmt->regs_addr;

    uint32_t be_value = be32toh(val);

    switch (offset) {
    case SH2E_CMT_CMSTR_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt_cmstr_reg_write(sh2e_cmt, (uint16_t) (be_value >> 16));
        sh2e_cmt_cmcsr_reg_write(&sh2e_cmt->cmt_regs.channels[0], (uint16_t) be_value);
        break;
    }
    case SH2E_CMT_CMCNT0_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt->cmt_regs.channels[0].cmcnt = (uint16_t) (be_value >> 16);
        sh2e_cmt->cmt_regs.channels[0].cmcor = (uint16_t) be_value;
        break;
    }
    case SH2E_CMT_CMCSR1_REGISTER_ADDRESS_OFFSET: {
        sh2e_cmt_cmcsr_reg_write(&sh2e_cmt->cmt_regs.channels[1], (uint16_t) (be_value >> 16));
        sh2e_cmt->cmt_regs.channels[1].cmcnt = (uint16_t) be_value;
        break;
    }
    case SH2E_CMT_CMCOR1_REGISTER_ADDRESS_OFFSET: {
        // This case is a bit special, as there is no register that would be written to together with CMCOR1.
        // So the upper half of the written value will be written to CMCOR1, and the lower half will be ignored.
        sh2e_cmt->cmt_regs.channels[1].cmcor = (uint16_t) (be_value >> 16);
        break;
    }
    }
}

static void update_channel_counter(sh2e_cmt_t *cmt, unsigned int channel_num)
{
    ASSERT(channel_num < SH2E_CMT_CHANNELS_COUNT);

    sh2e_cmt_channel_reg_t *channel_reg = &cmt->cmt_regs.channels[channel_num];

    uint16_t period = 0;
    switch ((channel_reg->cmcsr.cks1 << 1) | channel_reg->cmcsr.cks0) {
    case 0: {
        period = SH2E_CMT_PERIOD_INTERVAL_1;
        break;
    }
    case 1: {
        period = SH2E_CMT_PERIOD_INTERVAL_2;
        break;
    }
    case 2: {
        period = SH2E_CMT_PERIOD_INTERVAL_3;
        break;
    }
    case 3: {
        period = SH2E_CMT_PERIOD_INTERVAL_4;
        break;
    }
    }

    uint_fast16_t *counter = NULL;
    unsigned int *int_no = NULL;
    uint64_t *interrupts_count = NULL;

    switch (channel_num) {
    case 0: {
        counter = &cmt->counter0;
        int_no = &cmt->int_no_first_channel;
        interrupts_count = &cmt->interrupts_channel_0_count;
        break;
    }
    case 1: {
        counter = &cmt->counter1;
        int_no = &cmt->int_no_second_channel;
        interrupts_count = &cmt->interrupts_channel_1_count;
        break;
    }
    }

    ASSERT(counter != NULL);
    ASSERT(int_no != NULL);
    ASSERT(interrupts_count != NULL);

    (*counter) += cmt->cpu_cycles;

    while (*counter >= period) {
        *counter -= period;
        channel_reg->cmcnt++;

        if (channel_reg->cmcnt == channel_reg->cmcor) {
            channel_reg->cmcsr.cmf = 1; // Set compare match flag
            channel_reg->cmcnt = 0; // Reset counter to 0 on compare match

            if (channel_reg->cmcsr.cmie) { // If interrupt is enabled
                (*interrupts_count)++;
                cpu_interrupt_up(cmt->cpu, *int_no);
            }
        }
    }
}

/** CMT step implementation
 *
 * @param dev Device pointer
 *
 */
static void dsh2ecmt_step(device_t *dev)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    for (unsigned int i = 0; i < SH2E_CMT_CHANNELS_COUNT; ++i) {
        bool enabled = false;

        switch (i) {
        case 0: {
            enabled = sh2e_cmt->cmt_regs.cmstr.str0;
            break;
        }
        case 1: {
            enabled = sh2e_cmt->cmt_regs.cmstr.str1;
            break;
        }
        default: {
            break;
        }
        }

        if (enabled) {
            update_channel_counter(sh2e_cmt, i);
        }
    }

    sh2e_cmt->cpu_cycles = 0;
}

/** Dump CMT registers command. */
static bool
dsh2ecmt_cmd_dump_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_cmt_t *sh2e_cmt = device_get_sh2e_cmt(dev);

    printf("CMT\n");

    printf(" %6s: %04x\n", SH2E_CMT_CMSTR_NAME, sh2e_cmt->cmt_regs.cmstr.value);

    // General registers.
    for (unsigned int i = 0; i < SH2E_CMT_CHANNELS_COUNT; ++i) {
        uint16_t regs[] = {
            sh2e_cmt->cmt_regs.channels[i].cmcsr.value,
            sh2e_cmt->cmt_regs.channels[i].cmcnt,
            sh2e_cmt->cmt_regs.channels[i].cmcor
        };

        for (unsigned int j = 0; j < SH2E_CMT_CHANNEL_REGISTERS_COUNT; ++j) {
            printf(
                    " %5s%u: %04x\n",
                    sh2e_cmt_channel_reg_names[j],
                    i,
                    regs[j]);
        }
    }

    return true;
}

static bool dsh2ecmt_add_cpu(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    const char *cpu_name = parm_str_next(&parm);
    device_t *cpu_dev = dev_by_name(cpu_name);

    if (cpu_dev == NULL) {
        error("CPU device '%s' not found", cpu_name);
        return false;
    }

    if (is_dev_cpu(cpu_dev)) {
        general_cpu_t *cpu = (general_cpu_t *) cpu_dev->data;
        sh2e_cmt_t *cmt = device_get_sh2e_cmt(dev);
        cmt->cpu = cpu;
    } else {
        error("The device %s is not a CPU, it is a device of type %s.",
                cpu_dev->name, cpu_dev->type->name);
        return false;
    }

    return true;
}

/** Dispose dsh2ecmt
 *
 * @param dev Device pointer
 *
 */
static void dsh2ecmt_done(device_t *dev)
{
    ASSERT(dev != NULL);
    peripheral_t *peripheral = (peripheral_t *) dev->data;

    sh2e_cmt_t *sh2e_cmt = (sh2e_cmt_t *) peripheral->data;
    safe_free(sh2e_cmt);

    safe_free(peripheral);
}

static cmd_t dsh2ecmt_cmds[] = {
    { "init",
            (fcmd_t) dsh2ecmt_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/timer name" NEXT
                    REQ INT "channel 1 interrupt number" NEXT
                            REQ INT "channel 2 interrupt number" NEXT
                                    OPT INT "addr/register block address" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display help",
            "Display help",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dsh2ecmt_info,
            DEFAULT,
            DEFAULT,
            "Display device configuration",
            "Display device configuration",
            NOCMD },
    { "stat",
            (fcmd_t) dsh2ecmt_stat,
            DEFAULT,
            DEFAULT,
            "Display device statictics",
            "display device statictics",
            NOCMD },
    { "rd",
            (fcmd_t) dsh2ecmt_cmd_dump_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of CMT registers",
            "Dump contents of CMT registers",
            NOCMD },
    { "addcpu",
            (fcmd_t) dsh2ecmt_add_cpu,
            DEFAULT,
            DEFAULT,
            "Add CPU to the device",
            "Add CPU to the device",
            REQ STR "cpu name" END },
    LAST_CMD
};

device_type_t const dsh2ecmt = {
    .nondet = false,

    /* Type name and description */
    .name = "dsh2ecmt",
    .brief = "Compare Match Timer (CMT) device for SH-2E",
    .full = "This device simulates the on-chip Compare Match Timer (CMT) of the SH-2E microprocessor with 2 16-bit timer channels. The CMT has 16-bit counters and can generate interrupts at set intervals.",

    /* Functions */
    .read8 = dsh2ecmt_read8,
    .read16 = dsh2ecmt_read16,
    .read32 = dsh2ecmt_read32,

    .write8 = dsh2ecmt_write8,
    .write16 = dsh2ecmt_write16,
    .write32 = dsh2ecmt_write32,

    .step = dsh2ecmt_step,
    .done = dsh2ecmt_done,

    /* Commands */
    .cmds = dsh2ecmt_cmds
};
