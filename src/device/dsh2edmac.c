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
#include "../physmem.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "dsh2edmac.h"
#include "intc/superh_sh2e/intc.h"
#include "peripheral.h"

#define SH2E_DMAC_REGISTERS_START_ADDRESS UINT32_C(0xFFFFECB0)
#define SH2E_DMAC_REGISTERS_SIZE 0x50 /* Size of the DMAC registers. */

#define SH2E_DMAC_DMAOR_ADDRESS_OFFSET 0x0
#define SH2E_DMAC_CHANNELS_OFFSET 0x10
#define SH2E_DMAC_SAR_OFFSET_FROM_CHANNEL_START 0x0
#define SH2E_DMAC_DAR_OFFSET_FROM_CHANNEL_START 0x4
#define SH2E_DMAC_TCR_OFFSET_FROM_CHANNEL_START 0x8
#define SH2E_DMAC_CHCR_OFFSET_FROM_CHANNEL_START 0xC

#define SH2E_DMAC_CHANNEL_REGISTERS_COUNT 4
#define SH2E_DMAC_TCR_WRITE_MASK 0xFFFFFF /* Only bits 23 to 0 are writable in the TCR register. */
#define SH2E_DMAC_CHCR_WRITE_MASK 0x111F333D /* Bit 1 allows only a 0 write after reading 1 */
#define SH2E_DMAC_DMAOR_WRITE_MASK 0x0007 /* Only bits 2 to 0 are writable in the DMAOR register. */
#define SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE 4

#define SH2E_DMAC_TRANSFER_SIZE_8BIT 0
#define SH2E_DMAC_TRANSFER_SIZE_16BIT 1
#define SH2E_DMAC_TRANSFER_SIZE_32BIT 2

#define SH2E_DMAC_DMAOR_NAME "dmaor"

// Link to hardware manual (REJ09B0045-0200H, page 157)

char const *const sh2e_dmac_channel_reg_names[SH2E_DMAC_CHANNEL_REGISTERS_COUNT] = {
    "sar",
    "dar",
    "tcr",
    "chcr"
};

/**
 * @brief Merges a 16-bit value into a 32-bit register value according to the upper flag.
 * @param current The current 32-bit register value.
 * @param value The 16-bit value to merge.
 * @param upper If true, the value is merged into the upper 16 bits; otherwise, it is merged into the lower 16 bits.
 * @return The resulting 32-bit value after merging.
 */
static uint32_t merge_u16(uint32_t current, uint16_t value, bool upper)
{
    if (upper) {
        return (current & 0x0000FFFF) | ((uint32_t) value << 16);
    }

    return (current & 0xFFFF0000) | (uint32_t) value;
}

/**
 * @brief Extracts a 16-bit value from a 32-bit register value according to the upper flag.
 * @param value The 32-bit register value to extract from.
 * @param upper If true, the upper 16 bits are extracted; otherwise, the lower 16 bits are extracted.
 * @return The extracted 16-bit value.
 */
static uint16_t extract_u16(uint32_t value, bool upper)
{
    return upper ? (value >> 16) & 0xFFFF : value & 0xFFFF;
}

/**
 * @brief Gets a pointer to the specified register of a DMAC channel.
 * @param channel Pointer to the DMAC channel registers structure.
 * @param reg_no The register number to get the pointer for (0 for SAR, 1 for DAR, 2 for TCR, 3 for CHCR).
 * @return A pointer to the specified register, or NULL if the reg_no is invalid.
 */
static uint32_t *channel_reg_ptr(sh2e_dmac_channel_regs_t *channel, unsigned reg_no)
{
    switch (reg_no) {

    case SH2E_DMAC_SAR_OFFSET_FROM_CHANNEL_START:
        return &channel->sar;

    case SH2E_DMAC_DAR_OFFSET_FROM_CHANNEL_START:
        return &channel->dar;

    case SH2E_DMAC_TCR_OFFSET_FROM_CHANNEL_START:
        return &channel->tcr;

    case SH2E_DMAC_CHCR_OFFSET_FROM_CHANNEL_START:
        return &channel->chcr.value;

    default:
        return NULL;
    }
}

/**
 * @brief Write to the CHCR register
 * @param dmac Pointer to the DMAC instance
 * @param c_no Channel number (0 to 3)
 * @param val Value to write to the CHCR register
 */
static void sh2e_dmac_chcr_write(sh2e_dmac_t *dmac, unsigned int c_no, uint32_t val)
{
    sh2e_dmac_chcr_reg_t new_val = { .value = val };
    sh2e_dmac_chcr_reg_t *chcr = &dmac->dmac_regs.channels[c_no].chcr;

    /* Direct/Indirect Select bit is only writable for channel 3, for other channels it is always wired to 0 */
    chcr->di = (c_no == 3) ? new_val.di : 0;
    /* ro bit is writeable only for channel 2, for other channels it is always wired to 0 */
    chcr->ro = (c_no == 2) ? new_val.ro : 0;
    chcr->rs = new_val.rs;
    chcr->sm = new_val.sm;
    chcr->dm = new_val.dm;
    chcr->ts = new_val.ts;
    chcr->tm = new_val.tm;
    chcr->ie = new_val.ie;
    chcr->te = !new_val.te ? 0 : chcr->te; /* Allows only a 0 write after reading 1 */
    chcr->de = new_val.de;

    if (!chcr->de && c_no == 2) {
        dmac->reload_counter = SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE;
    }
}

/**
 * @brief Resets the DMAC to its initial state, clearing all registers and resetting internal state variables.
 * This function is called on power-on reset, sleep mode, hardware standby mode, and software standby mode.
 * @param dmac Pointer to the DMAC instance to reset.
 */
static void sh2e_dmac_reset(sh2e_dmac_t *dmac)
{
    memset(&dmac->dmac_regs, 0, sizeof(dmac->dmac_regs));
    dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_INITIAL;
    dmac->picked_channel = -1;
    dmac->reload_counter = SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE;
}

/**
 * @brief Interrupt handler function for the DMAC peripheral.
 * @param peripheral Pointer to the peripheral structure which contains the DMAC instance data.
 * @param int_no The interrupt number that is being signaled.
 */
static void sh2e_dmac_interrupt_up(void *peripheral, unsigned int int_no)
{
    sh2e_dmac_t *dmac = (sh2e_dmac_t *) ((peripheral_t *) peripheral)->data;

    switch (int_no) {
    case SH2E_INTC_POWER_ON_RESET_EXTERNAL_OFFSET:
    case SH2E_INTC_POWER_ON_RESET_INTERNAL_OFFSET:
    case SH2E_INTC_SLEEP_MODE_OFFSET:
    case SH2E_INTC_HARDWARE_STANBY_MODE_OFFSET:
    case SH2E_INTC_SOFTWARE_STANBY_MODE_OFFSET: {
        sh2e_dmac_reset(dmac);
        break;
    }
    case SH2E_INTC_NMI_VECTOR_ADDRESS_OFFSET: {
        dmac->dmac_regs.dmaor.nmif = 1;
        break;
    }
    }
}

/**
 * @brief Update cycles handler function for the DMAC peripheral, called by the CPU to notify the DMAC of cycle updates.
 * @param peripheral Pointer to the peripheral structure which contains the DMAC instance data.
 * @param cycles The number of CPU cycles since the last update.
 */
static void sh2e_dmac_cpu_cycles_update(void *peripheral, unsigned int cycles)
{
    sh2e_dmac_t *dmac = (sh2e_dmac_t *) ((peripheral_t *) peripheral)->data;
    dmac->cpu_cycles = cycles;
}

static peripheral_ops_t const sh2e_dmac_peripheral_ops = {
    .interrupt_up = (interrupt_func_t) sh2e_dmac_interrupt_up,
    .update_cycles = (update_cycles_func_t) sh2e_dmac_cpu_cycles_update
};

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dsh2edmac_init(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    parm_next(&parm);

    uint64_t int_nos[SH2E_DMAC_CHANNELS_COUNT];

    for (unsigned int i = 0; i < SH2E_DMAC_CHANNELS_COUNT; i++) {
        int_nos[i] = parm_uint_next(&parm);
    }

    uint64_t _addr;
    if (parm_type(parm) == tt_end) {
        _addr = SH2E_DMAC_REGISTERS_START_ADDRESS;
    } else {
        _addr = parm_uint_next(&parm);
    }

    if (!phys_range(_addr)) {
        error("Physical memory address out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) SH2E_DMAC_REGISTERS_SIZE)) {
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
    sh2e_dmac_t *sh2e_dmac = safe_malloc_t(sh2e_dmac_t);
    sh2e_dmac->regs_addr = addr;

    sh2e_dmac_reset(sh2e_dmac);

    for (unsigned int i = 0; i < SH2E_DMAC_CHANNELS_COUNT; i++) {
        sh2e_dmac->interrupt_number[i] = int_nos[i];
    }

    peripheral_t *generic_peripheral = safe_malloc_t(peripheral_t);
    *generic_peripheral = (peripheral_t) {
        .data = sh2e_dmac,
        .type = &sh2e_dmac_peripheral_ops,
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
static bool dsh2edmac_info(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    printf("SH-2E DMAC\n");
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
static bool dsh2edmac_stat(token_t *parm, device_t *dev)
{
    ASSERT(dev != NULL);

    // TODO:
    // sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);

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
static void dsh2edmac_read8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t *val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *sh2e_dmac = device_get_sh2e_dmac(dev);

    ptr36_t dmac_start = sh2e_dmac->regs_addr;
    ptr36_t dmac_end = sh2e_dmac->regs_addr + SH2E_DMAC_REGISTERS_SIZE;

    if (addr >= dmac_start && addr < dmac_end) {
        error("Reading by a byte transfer function is not supported. Registers must be read by word or longword transfer functions.");
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
static void dsh2edmac_read16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t *val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);
    ptr36_t offset = addr - dmac->regs_addr;

    if (offset >= SH2E_DMAC_REGISTERS_SIZE) {
        return;
    }

    /* DMAOR */
    if (offset == SH2E_DMAC_DMAOR_ADDRESS_OFFSET) {
        *val = htobe16(dmac->dmac_regs.dmaor.value);
        return;
    }

    if (offset < SH2E_DMAC_CHANNELS_OFFSET) {
        return;
    }

    offset -= SH2E_DMAC_CHANNELS_OFFSET;

    unsigned int channel_no = offset / sizeof(sh2e_dmac_channel_regs_t);
    unsigned int reg_no = offset % sizeof(sh2e_dmac_channel_regs_t);
    bool upper = (reg_no % 4) == 0;

    sh2e_dmac_channel_regs_t *ch = &dmac->dmac_regs.channels[channel_no];

    // Mask out bit 1 to treat upper and lower word reads the same, as the registers are 32-bit but accessed by 16-bit transfers
    uint32_t *reg_ptr = channel_reg_ptr(ch, reg_no & ~0x2);

    if (reg_ptr == NULL) {
        return;
    }

    *val = htobe16(extract_u16(*reg_ptr, upper));
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
static void dsh2edmac_read32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t *val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);
    ptr36_t offset = addr - dmac->regs_addr;

    if (offset >= SH2E_DMAC_REGISTERS_SIZE) {
        return;
    }

    /* DMAOR */
    if (offset == SH2E_DMAC_DMAOR_ADDRESS_OFFSET) {
        error("Reading by a longword transfer function to DMAOR register is not supported. DMAOR register must be read by word transfer function.");
        return;
    }

    if (offset < SH2E_DMAC_CHANNELS_OFFSET) {
        return;
    }

    offset -= SH2E_DMAC_CHANNELS_OFFSET;
    unsigned int channel_no = offset / sizeof(sh2e_dmac_channel_regs_t);
    unsigned int reg_no = offset % sizeof(sh2e_dmac_channel_regs_t);

    uint32_t *reg_ptr = channel_reg_ptr(&dmac->dmac_regs.channels[channel_no], reg_no);

    if (reg_ptr == NULL) {
        return;
    }

    *val = htobe32(*reg_ptr);
}

/** Write byte command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2edmac_write8(unsigned int procno, device_t *dev, ptr36_t addr, uint8_t val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *sh2e_dmac = device_get_sh2e_dmac(dev);

    ptr36_t dmac_start = sh2e_dmac->regs_addr;
    ptr36_t dmac_end = sh2e_dmac->regs_addr + SH2E_DMAC_REGISTERS_SIZE;

    if (addr >= dmac_start && addr < dmac_end) {
        error("Writing by a byte transfer function is not supported. Registers must be written to by word or longword transfer functions.");
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
static void dsh2edmac_write16(unsigned int procno, device_t *dev, ptr36_t addr, uint16_t val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);

    ptr36_t offset = addr - dmac->regs_addr;

    if (offset >= SH2E_DMAC_REGISTERS_SIZE) {
        return;
    }

    uint16_t be_value = be16toh(val);

    /* DMAOR */
    if (offset == SH2E_DMAC_DMAOR_ADDRESS_OFFSET) {
        dmac->dmac_regs.dmaor.value = be_value & SH2E_DMAC_DMAOR_WRITE_MASK;
        return;
    }

    if (offset < SH2E_DMAC_CHANNELS_OFFSET) {
        return;
    }

    offset -= SH2E_DMAC_CHANNELS_OFFSET;

    unsigned int channel_no = offset / sizeof(sh2e_dmac_channel_regs_t);
    unsigned int reg_no = offset % sizeof(sh2e_dmac_channel_regs_t);
    bool upper = (reg_no % 4) == 0;

    sh2e_dmac_channel_regs_t *ch = &dmac->dmac_regs.channels[channel_no];

    // Mask out bit 1 to treat upper and lower word writes the same, as the registers are 32-bit but accessed by 16-bit transfers
    switch (reg_no & ~0x2) {

    case SH2E_DMAC_SAR_OFFSET_FROM_CHANNEL_START: {
        ch->sar = merge_u16(ch->sar, be_value, upper);
        break;
    }

    case SH2E_DMAC_DAR_OFFSET_FROM_CHANNEL_START: {
        ch->dar = merge_u16(ch->dar, be_value, upper);
        break;
    }

    case SH2E_DMAC_TCR_OFFSET_FROM_CHANNEL_START: {
        uint32_t new_tcr = merge_u16(ch->tcr, be_value, upper);
        ch->tcr = new_tcr & SH2E_DMAC_TCR_WRITE_MASK;
        break;
    }

    case SH2E_DMAC_CHCR_OFFSET_FROM_CHANNEL_START: {
        uint32_t new_chcr = merge_u16(ch->chcr.value, be_value, upper);
        sh2e_dmac_chcr_write(dmac, channel_no, new_chcr);
        break;
    }

    default: {
        break;
    }
    }
}

/** Write longword command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2edmac_write32(unsigned int procno, device_t *dev, ptr36_t addr, uint32_t val)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);
    ptr36_t offset = addr - dmac->regs_addr;

    if (offset >= SH2E_DMAC_REGISTERS_SIZE) {
        return;
    }

    // The DMAC DMAOR register
    if (offset == SH2E_DMAC_DMAOR_ADDRESS_OFFSET) {
        error("Writing by a longword transfer function to DMAOR register is not supported. DMAOR register must be written to by word transfer function.");
        return;
    }

    if (offset < SH2E_DMAC_CHANNELS_OFFSET) {
        return;
    }

    offset -= SH2E_DMAC_CHANNELS_OFFSET;
    unsigned int channel_no = offset / sizeof(sh2e_dmac_channel_regs_t);
    unsigned int reg_no = offset % sizeof(sh2e_dmac_channel_regs_t);

    uint32_t be_value = be32toh(val);

    switch (reg_no) {
    case SH2E_DMAC_SAR_OFFSET_FROM_CHANNEL_START: {
        dmac->dmac_regs.channels[channel_no].sar = be_value;
        break;
    }
    case SH2E_DMAC_DAR_OFFSET_FROM_CHANNEL_START: {
        dmac->dmac_regs.channels[channel_no].dar = be_value;
        break;
    }
    case SH2E_DMAC_TCR_OFFSET_FROM_CHANNEL_START: {
        dmac->dmac_regs.channels[channel_no].tcr = be_value & SH2E_DMAC_TCR_WRITE_MASK;
        break;
    }
    case SH2E_DMAC_CHCR_OFFSET_FROM_CHANNEL_START: {
        sh2e_dmac_chcr_write(dmac, channel_no, be_value);
        break;
    }
    default: {
        return;
    }
    }
}

static void byte_transfer(sh2e_dmac_t *dmac, uint32_t const s_addr, uint32_t const d_addr)
{
    uint8_t data = physmem_read8(dmac->cpu->cpuno, s_addr, true);
    physmem_write8(dmac->cpu->cpuno, d_addr, data, true);
}

static void word_transfer(sh2e_dmac_t *dmac, uint32_t const s_addr, uint32_t const d_addr)
{
    if (s_addr & 1) {
        dmac->dmac_regs.dmaor.ae = 1;
        cpu_interrupt_up(dmac->cpu, SH2E_INTC_DMAC_VECTOR_ADDRESS_OFFSET);
        return;
    }
    uint32_t data = physmem_read16(dmac->cpu->cpuno, s_addr, true);
    physmem_write16(dmac->cpu->cpuno, d_addr, data, true);
}

static void longword_transfer(sh2e_dmac_t *dmac, uint32_t const s_addr, uint32_t const d_addr)
{
    if (s_addr & 3) {
        dmac->dmac_regs.dmaor.ae = 1;
        cpu_interrupt_up(dmac->cpu, SH2E_INTC_DMAC_VECTOR_ADDRESS_OFFSET);
        return;
    }
    uint32_t data = physmem_read32(dmac->cpu->cpuno, s_addr, true);
    physmem_write32(dmac->cpu->cpuno, d_addr, data, true);
}

static void regs_update_after_transfer(sh2e_dmac_t *dmac, unsigned int channel_num)
{
    sh2e_dmac_channel_regs_t *regs = &dmac->dmac_regs.channels[channel_num];
    uint32_t transfer_size_bytes = 1 << regs->chcr.ts;

    // Update SAR
    if (regs->chcr.sm == 1) {
        regs->sar += regs->chcr.di ? 4 : transfer_size_bytes; // Indirect mode always increments by 4 bytes
    } else if (regs->chcr.sm == 2) {
        regs->sar -= regs->chcr.di ? 4 : transfer_size_bytes; // Indirect mode always decrements by 4 bytes
    }

    // Update DAR
    if (regs->chcr.dm == 1) {
        regs->dar += transfer_size_bytes;
    } else if (regs->chcr.dm == 2) {
        regs->dar -= transfer_size_bytes;
    }
}

static void perform_transfer(sh2e_dmac_t *dmac, unsigned int c_no)
{
    sh2e_dmac_channel_regs_t *channel_regs = &dmac->dmac_regs.channels[c_no];

    uint32_t s_addr;
    uint32_t d_addr = channel_regs->dar; // Destination address is always taken from the DAR register

    if (channel_regs->chcr.di == 0) {
        s_addr = channel_regs->sar;
    } else {
        // Indirect address mode (only for channel 3)
        s_addr = physmem_read32(dmac->cpu->cpuno, channel_regs->sar, true);
    }

    // Perform transfers
    switch (channel_regs->chcr.ts) {
    case SH2E_DMAC_TRANSFER_SIZE_8BIT: {
        byte_transfer(dmac, s_addr, d_addr);
        break;
    }
    case SH2E_DMAC_TRANSFER_SIZE_16BIT: {
        word_transfer(dmac, s_addr, d_addr);
        break;
    }
    case SH2E_DMAC_TRANSFER_SIZE_32BIT: {
        longword_transfer(dmac, s_addr, d_addr);
        break;
    }
    default: {
        ASSERT(false && "Invalid transfer size");
    }
    }

    regs_update_after_transfer(dmac, c_no);

    --dmac->dmac_regs.channels[c_no].tcr;
}

static void transfer_and_update_regs(sh2e_dmac_t *dmac)
{
    ASSERT(dmac->picked_channel != -1);

    perform_transfer(dmac, dmac->picked_channel);

    // Update reload counter for channel 2 and reload SAR if needed
    if (dmac->picked_channel == 2) {
        --dmac->reload_counter;
        if (dmac->reload_counter == 0) {
            dmac->dmac_regs.channels[dmac->picked_channel].sar = dmac->initial_sar2; // Reload SAR for channel 2
            dmac->reload_counter = SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE;
        }
    }

    // Check if the transfer is finished (TCR reached 0)
    if (dmac->dmac_regs.channels[dmac->picked_channel].tcr == 0) {
        // Set transfer end bit
        dmac->dmac_regs.channels[dmac->picked_channel].chcr.te = 1;
        if (dmac->picked_channel == 2) {
            dmac->reload_counter = SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE;
        }
        if (dmac->dmac_regs.channels[dmac->picked_channel].chcr.ie) {
            //  Interrupt request if ie is set
            cpu_interrupt_up(dmac->cpu, dmac->interrupt_number[dmac->picked_channel]);
        }
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_INITIAL;
        return;
    }

    // Check if the transfer should be aborted due to DMAOR conditions
    if (dmac->dmac_regs.dmaor.nmif || dmac->dmac_regs.dmaor.ae || !dmac->dmac_regs.dmaor.dme || !dmac->dmac_regs.channels[dmac->picked_channel].chcr.de) {
        // Abort the transfer
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_INITIAL;
        if (dmac->picked_channel == 2) {
            dmac->reload_counter = SH2E_DMAC_RELOAD_COUNTER_INITIAL_VALUE;
        }
        return;
    }

    // Burst-mode keeps transferring for one more transfer
    if (dmac->dmac_regs.channels[dmac->picked_channel].chcr.tm) {
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_TRANSFERRING;
    } else {
        // Otherwise we wait for the next request
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_WAITING_FOR_REQUEST;
    }
}

static void step_transferring(sh2e_dmac_t *dmac)
{
    ASSERT(dmac->picked_channel != -1);

    transfer_and_update_regs(dmac);

    // We simulate the burst-mode transfer by performing 2 transfers instead of 1 in each step
    if (dmac->transfer_state == SH2E_DMAC_TRANSFER_STATE_TRANSFERRING) {
        transfer_and_update_regs(dmac);
        // But we stop the burst after 2 transfers and wait for the next request
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_WAITING_FOR_REQUEST;
    }
}

static void step_waiting(sh2e_dmac_t *dmac)
{
    ASSERT(dmac->picked_channel != -1);

    // TODO: implement peripheral requests
    switch (dmac->dmac_regs.channels[dmac->picked_channel].chcr.rs) {
    case 0b11111: {
        // Auto-request - trasfer immediately
        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_TRANSFERRING;
        step_transferring(dmac);
        break;
    }
    default: {
        break;
    }
    }
}

static void step_initial(sh2e_dmac_t *dmac)
{
    dmac->picked_channel = -1;
    if (dmac->dmac_regs.dmaor.dme && !dmac->dmac_regs.dmaor.nmif && !dmac->dmac_regs.dmaor.ae) {
        for (unsigned int i = 0; i < SH2E_DMAC_CHANNELS_COUNT; ++i) {
            sh2e_dmac_channel_regs_t *channel_regs = &dmac->dmac_regs.channels[i];

            if (channel_regs->chcr.de && !channel_regs->chcr.te) {
                dmac->picked_channel = i;
                break;
            }
        }
        // No channel enabled for transfer
        if (dmac->picked_channel == -1) {
            return;
        }

        if (dmac->picked_channel == 2) {
            dmac->initial_sar2 = dmac->dmac_regs.channels[2].sar;
        }

        dmac->transfer_state = SH2E_DMAC_TRANSFER_STATE_WAITING_FOR_REQUEST;
        step_waiting(dmac);
    }
}

/** DMAC step implementation
 *
 * @param dev Device pointer
 *
 */
static void dsh2edmac_step(device_t *dev)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);

    while (dmac->cpu_cycles > 0) {
        switch (dmac->transfer_state) {
        case SH2E_DMAC_TRANSFER_STATE_INITIAL: {
            step_initial(dmac);
            break;
        }
        case SH2E_DMAC_TRANSFER_STATE_WAITING_FOR_REQUEST: {
            step_waiting(dmac);
            break;
        }
        case SH2E_DMAC_TRANSFER_STATE_TRANSFERRING: {
            step_transferring(dmac);
            break;
        }
        default: {
            ASSERT(false && "Invalid DMAC transfer state");
        }
        }
        --dmac->cpu_cycles;
    }
}

/** Dump DMAC registers command. */
static bool
dsh2edmac_cmd_dump_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);

    printf("DMAC\n");

    for (unsigned int i = 0; i < SH2E_DMAC_CHANNELS_COUNT; ++i) {
        uint32_t regs[] = {
            dmac->dmac_regs.channels[i].sar,
            dmac->dmac_regs.channels[i].dar,
            dmac->dmac_regs.channels[i].tcr,
            dmac->dmac_regs.channels[i].chcr.value
        };

        for (unsigned int j = 0; j < sizeof(regs) / sizeof(regs[0]); ++j) {
            printf(
                    " %5s%u: %08x\n",
                    sh2e_dmac_channel_reg_names[j],
                    i,
                    regs[j]);
        }
    }

    printf(" %6s: %04x\n", SH2E_DMAC_DMAOR_NAME, dmac->dmac_regs.dmaor.value);

    return true;
}

static bool dsh2edmac_cmd_add_cpu(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    const char *cpu_name = parm_str_next(&parm);
    device_t *cpu_dev = dev_by_name(cpu_name);

    if (cpu_dev == NULL) {
        error("CPU device '%s' not found", cpu_name);
        return false;
    }

    sh2e_dmac_t *dmac = device_get_sh2e_dmac(dev);
    general_cpu_t *cpu = cpu_dev->data;

    dmac->cpu = cpu;

    return true;
}

/** Dispose dsh2edmac
 *
 * @param dev Device pointer
 *
 */
static void dsh2edmac_done(device_t *dev)
{
    ASSERT(dev != NULL);

    safe_free(dev->data);
}

static cmd_t dsh2edmac_cmds[] = {
    { "init",
            (fcmd_t) dsh2edmac_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/dmac name" NEXT
                    REQ INT "interrupt number channel 0" NEXT
                            REQ INT "interrupt number channel 1" NEXT
                                    REQ INT "interrupt number channel 2" NEXT
                                            REQ INT "interrupt number channel 3" NEXT
                                                    OPT INT "addr/register block address" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display help",
            "Display help",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dsh2edmac_info,
            DEFAULT,
            DEFAULT,
            "Display device configuration",
            "Display device configuration",
            NOCMD },
    { "stat",
            (fcmd_t) dsh2edmac_stat,
            DEFAULT,
            DEFAULT,
            "Display device statictics",
            "display device statictics",
            NOCMD },
    { "rd",
            (fcmd_t) dsh2edmac_cmd_dump_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of DMAC registers",
            "Dump contents of DMAC registers",
            NOCMD },

    { "addcpu",
            (fcmd_t) dsh2edmac_cmd_add_cpu,
            DEFAULT,
            DEFAULT,
            "Add CPU to the device",
            "Add CPU to the device",
            REQ STR "cpu name" END },
    LAST_CMD
};

device_type_t const dsh2edmac = {
    .nondet = false,

    /* Type name and description */
    .name = "dsh2edmac",
    .brief = "Direct Memory Access Controller (DMAC) device for SH-2E",
    .full = "This device simulates the on-chip Direct Memory Access Controller (DMAC) of the SH-2E microprocessor.",

    .read8 = dsh2edmac_read8,
    .read16 = dsh2edmac_read16,
    .read32 = dsh2edmac_read32,

    .write8 = dsh2edmac_write8,
    .write16 = dsh2edmac_write16,
    .write32 = dsh2edmac_write32,

    .step = dsh2edmac_step,
    .done = dsh2edmac_done,

    /* Commands */
    .cmds = dsh2edmac_cmds
};
