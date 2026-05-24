/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E Direct Memory Access Controller device.
 *
 */

#ifndef DSH2E_DMAC_H_
#define DSH2E_DMAC_H_

#include "device.h"

#define PACKED __attribute__((packed))
#define device_get_sh2e_dmac(dev) ((sh2e_dmac_t *) (((peripheral_t *) (dev)->data)->data))

#define SH2E_DMAC_CHANNELS_COUNT 4
#define SH2E_DMAC_PERIPHERAL_REQUESTS_COUNT 32 /* there are 5 rs bits -> 32 possible values */

extern device_type_t const dsh2edmac;

typedef struct sh2e_cpu sh2e_cpu_t;

typedef union sh2e_dmac_chcr_reg {
    uint32_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint32_t _rf0 : 3; /** Reserved field 0 (bits 31 to 29), wired to 0. */
        uint32_t di : 1; /** Direct/Indirect Select bit (bit 28) */
        uint32_t _rf1 : 3; /** Reserved field 1 (bits 27 to 25), wired to 0. */
        uint32_t ro : 1; /** Source Address Reload (bit 24) */
        uint32_t _rf2 : 3; /** Reserved field 2 (bits 23 to 21), wired to 0. */
        uint32_t rs : 5; /** Register Select bits (bits 20 to 16). */
        uint32_t _rf3 : 2; /** Reserved field 3 (bits 15 to 14), wired to 0. */
        uint32_t sm : 2; /** Source Address Mode bits (bits 13 to 12). */
        uint32_t _rf4 : 2; /** Reserved field 4 (bits 11 to 10), wired to 0. */
        uint32_t dm : 2; /** Destination Address Mode bits (bits 9 to 8). */
        uint32_t _rf5 : 2; /** Reserved field 5 (bits 7 to 6), wired to 0. */
        uint32_t ts : 2; /** Transfer Size bits (bits 5 to 4). */
        uint32_t tm : 1; /** Transfer Mode bit (bit 3). */
        uint32_t ie : 1; /** Interrupt Enable bit (bit 2). */
        uint32_t te : 1; /** Transfer End bit (bit 1). */
        uint32_t de : 1; /** DMAC Enable bit (bit 0). */
#else
        uint32_t de : 1; /** DMAC Enable bit (bit 0). */
        uint32_t te : 1; /** Transfer End bit (bit 1). */
        uint32_t ie : 1; /** Interrupt Enable bit (bit 2). */
        uint32_t tm : 1; /** Transfer Mode bit (bit 3). */
        uint32_t ts : 2; /** Transfer Size bits (bits 5 to 4). */
        uint32_t _rf5 : 2; /** Reserved field 5 (bits 7 to 6), wired to 0. */
        uint32_t dm : 2; /** Destination Address Mode bits (bits 9 to 8). */
        uint32_t _rf4 : 2; /** Reserved field 4 (bits 11 to 10), wired to 0. */
        uint32_t sm : 2; /** Source Address Mode bits (bits 13 to 12). */
        uint32_t _rf3 : 2; /** Reserved field 3 (bits 15 to 14), wired to 0. */
        uint32_t rs : 5; /** Resource Select bits (bits 20 to 16). */
        uint32_t _rf2 : 3; /** Reserved field 2 (bits 23 to 21), wired to 0. */
        uint32_t ro : 1; /** Source Address Reload (bit 24) */
        uint32_t _rf1 : 3; /** Reserved field 1 (bits 27 to 25), wired to 0. */
        uint32_t di : 1; /** Direct/Indirect Select bit (bit 28) */
        uint32_t _rf0 : 3; /** Reserved field 0 (bits 31 to 29), wired to 0. */
#endif
    };
} sh2e_dmac_chcr_reg_t;

typedef union sh2e_dmaor {
    uint16_t value;

    PACKED struct {
#ifdef WORDS_BIGENDIAN
        uint16_t _rf : 13; /** Reserved field (bits 15 to 3), wired to 0. */
        uint16_t ae : 1; /** Address Error flag (bit 2). */
        uint16_t nmif : 1; /** NMI Flag (bit 1). */
        uint16_t dme : 1; /** DMAC Master Enable bit (bit 0). */
#else
        uint16_t dme : 1; /** DMAC Master Enable bit (bit 0). */
        uint16_t nmif : 1; /** NMI Flag (bit 1). */
        uint16_t ae : 1; /** Address Error flag (bit 2). */
        uint16_t _rf : 13; /** Reserved field (bits 15 to 3), wired to 0. */
#endif
    };
} sh2e_dmaor_reg_t;

typedef struct sh2e_dmac_channel_regs {
    uint32_t sar; /* Source Address Register */
    uint32_t dar; /* Destination Address Register */
    uint32_t tcr; /* Transfer Count Register */
    sh2e_dmac_chcr_reg_t chcr; /* Channel Control Register */
} sh2e_dmac_channel_regs_t;

typedef struct sh2e_dmac_regs {
    sh2e_dmaor_reg_t dmaor; /* DMAC Operation Register */

    sh2e_dmac_channel_regs_t channels[SH2E_DMAC_CHANNELS_COUNT]; /* Registers for each channel. */
} sh2e_dmac_regs_t;

typedef enum sh2e_dmac_transfer_state {
    SH2E_DMAC_TRANSFER_STATE_INITIAL,
    SH2E_DMAC_TRANSFER_STATE_WAITING_FOR_REQUEST,
    SH2E_DMAC_TRANSFER_STATE_TRANSFERRING,
} sh2e_dmac_transfer_state_t;

typedef enum sh2e_dmac_peripheral_request_type {
    RECEIVE,
    TRANSMIT,
    DO_NOT_CARE,
    __SH2E_DMAC_PERIPHERAL_REQUEST_TYPE_COUNT,
} sh2e_dmac_peripheral_request_type_t;

typedef struct sh2e_dmac_peripheral_request_table_entry {
    uint8_t index; /* Index of the request that corresponds to the RS bits in the CHCR */
    bool pending; /* Whether the DMAC has received the request for this entry */
    bool registered; /* Whether this entry is registered by a peripheral via config */
    sh2e_dmac_peripheral_request_type_t type; /* Whether this entry corresponds to a receive data, transmit data, or if the type doesn't matter */

    /**
     * If the type is:
     * - RECEIVE: transfer is valid and will proceed only if the SAR contains the same address as the peripheral_request_address
     * - TRANSMIT: transfer is valid and will proceed only if the DAR contains the same address as the peripheral_request_address
     * - DO_NOT_CARE: transfer is valid regardless of the peripheral_request_address
     */
    uint32_t peripheral_request_address;

} sh2e_dmac_peripheral_request_table_entry_t;

typedef struct sh2e_dmac {
    general_cpu_t *cpu; /* Pointer to the CPU that the DMAC is attached to */

    uint64_t regs_addr; /* Base address of the DMAC registers */

    sh2e_dmac_regs_t dmac_regs; /* DMAC registers */

    unsigned int reload_counter; /* Counter used for reloading source address in channel 2 */

    unsigned int interrupt_number[SH2E_DMAC_CHANNELS_COUNT]; /* Interrupt number for each channel */

    unsigned int cpu_cycles; /* Number of CPU cycles since last tick */

    sh2e_dmac_transfer_state_t transfer_state; /* Current state of the ongoing transfer, if any */

    int picked_channel; /* Currently picked channel for transfer, -1 if no channel is picked */

    uint32_t initial_sar2; /* Initial SAR value at the start of transfer for channel 2, used for reloads */

    sh2e_dmac_peripheral_request_table_entry_t peripheral_request_table[SH2E_DMAC_PERIPHERAL_REQUESTS_COUNT]; /* Table for tracking pending requests from peripherals */

    uint64_t successful_transfers_count; /* Total number of successful transfers completed by the DMAC */
    uint64_t interrupts_count; /* Total number of interrupts triggered by the DMAC */
} sh2e_dmac_t;

#endif
