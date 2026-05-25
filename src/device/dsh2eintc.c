
#include <inttypes.h>

#include "../assert.h"
#include "../fault.h"
#include "../utils.h"
#include "device.h"
#include "dsh2eintc.h"
#include "intc/general_intc.h"
#include "intc/superh_sh2e/debug.h"
#include "intc/superh_sh2e/intc.h"

#define get_sh2e_intc(dev) ((sh2e_intc_t *) (((general_intc_t *) (dev)->data)->data))

static intc_ops_t const sh2e_intc_ops = {
    .interrupt_up = (interrupt_func_t) sh2e_intc_assert_interrupt,
    .interrupt_down = (interrupt_func_t) sh2e_intc_deassert_interrupt,

    .check_interrupts = (check_interrupts_func_t) sh2e_check_pending_interrupts,
    .accept_interrupt = (accept_interrupt_func_t) sh2e_accept_interrupt,

    .check_resets = (check_resets_func_t) sh2e_check_pending_resets,
    .accept_reset = (accept_reset_func_t) sh2e_accept_reset,

    .init = (init_intc_func_t) sh2e_intc_init_regs,
};

/** INTC initialization */
static bool
dsh2eintc_cmd_init(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    parm_next(&parm);

    uint64_t _addr;
    if (parm_type(parm) == tt_end) {
        _addr = SH2E_INTC_REGISTERS_START_ADDRESS;
    } else {
        _addr = parm_uint_next(&parm);
    }

    unsigned int id = get_free_intcno();
    if (id == MAX_INTCS) {
        error("Maximum INTC count exceeded (%u)", MAX_INTCS);
        return false;
    }

    if (!phys_range(_addr)) {
        error("Physical memory address out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) ((SH2E_INTC_IPR_REGISTERS_COUNT + SH2E_INTC_SYSTEM_REGISTERS_COUNT) * sizeof(uint16_t)))) {
        error("Invalid address, registers would exceed the physical "
              "memory range");
        return false;
    }

    ptr36_t addr = _addr;

    if (!ptr36_dword_aligned(addr)) {
        error("Physical memory address must be 8-byte aligned");
        return false;
    }

    sh2e_intc_t *sh2e_intc = safe_malloc_t(sh2e_intc_t);
    sh2e_intc_init(sh2e_intc, id, addr);

    general_intc_t *generic_intc = safe_malloc_t(general_intc_t);
    *generic_intc = (general_intc_t) {
        .intcno = id,
        .data = sh2e_intc,
        .type = &sh2e_intc_ops
    };

    add_intc(generic_intc);

    dev->data = generic_intc;

    return true;
}

/** INTC cleanup */
static void
dsh2eintc_done(device_t *const dev)
{
    ASSERT(dev != NULL);

    general_intc_t *generic_intc = (general_intc_t *) dev->data;

    sh2e_intc_t *sh2e_intc = (sh2e_intc_t *) generic_intc->data;
    sh2e_intc_done(sh2e_intc);
    safe_free(sh2e_intc);

    safe_free(generic_intc);
}

/** INTC info command. */
static bool
dsh2eintc_cmd_info(token_t *const parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    printf("SH-2E INTC\n");
    return true;
}

/** Stat command implementation
 *
 */
static bool dsh2eintc_cmd_stat(token_t *parm, device_t *dev)
{
    sh2e_intc_t *intc = get_sh2e_intc(dev);

    printf("[Number of accepted interrupts] [Number of accepted resets    ] [Total accepted               ]\n");
    printf("%31" PRIu64 " %31" PRIu64 " %31" PRIu64 "\n\n",
            (uint64_t) intc->accepted_interrupts,
            (uint64_t) intc->accepted_resets,
            (uint64_t) intc->accepted_interrupts + (uint64_t) intc->accepted_resets);

    return true;
}

/** Add interrupt source command. */
static bool
dsh2eintc_cmd_add_interrupt_source(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    uint16_t source_id = parm_uint_next(&parm);
    uint8_t priority_pool_index = parm_uint_next(&parm);
    uint32_t priority = parm_uint_next(&parm);

    sh2e_intc_add_interrupt_source(get_sh2e_intc(dev), source_id, priority_pool_index, priority);

    return true;
}

/** Read command implementation
 *
 * @param procno Processor number
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Pointer to store the read value
 *
 * @return Read value
 *
 */
static void dsh2eintc_read16(unsigned int procno, struct device *dev, ptr36_t addr, uint16_t *val)
{
    general_intc_t *generic_intc = (general_intc_t *) dev->data;
    sh2e_intc_t *sh2e_intc = (sh2e_intc_t *) generic_intc->data;

    switch (addr - sh2e_intc->regs_addr) {
    case SH2E_INTC_IPRA_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRB_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRC_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRD_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRE_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRF_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRG_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRH_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRI_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRJ_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRK_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRL_REGISTER_ADDRESS_OFFSET: {
        uint8_t index = (addr - sh2e_intc->regs_addr) / sizeof(uint16_t);
        *val = sh2e_intc_priority_register_read(sh2e_intc, index);
        break;
    }
    case SH2E_INTC_ICR_REGISTER_ADDRESS_OFFSET:
        *val = sh2e_intc_icr_reg_read(sh2e_intc).value;
        break;
    case SH2E_INTC_ISR_REGISTER_ADDRESS_OFFSET:
        *val = sh2e_intc_isr_reg_read(sh2e_intc).value;
        break;
    }
}

/** Write command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dsh2eintc_write16(unsigned int procno, device_t *dev, ptr36_t addr,
        uint16_t val)
{
    general_intc_t *generic_intc = (general_intc_t *) dev->data;
    sh2e_intc_t *sh2e_intc = (sh2e_intc_t *) generic_intc->data;

    switch (addr - sh2e_intc->regs_addr) {
    case SH2E_INTC_IPRA_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRB_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRC_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRD_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRE_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRF_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRG_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRH_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRI_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRJ_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRK_REGISTER_ADDRESS_OFFSET:
    case SH2E_INTC_IPRL_REGISTER_ADDRESS_OFFSET: {
        uint8_t index = (addr - sh2e_intc->regs_addr) / sizeof(uint16_t);
        sh2e_intc_priority_register_write(sh2e_intc, index, val);
        break;
    }
    case SH2E_INTC_ICR_REGISTER_ADDRESS_OFFSET:
        sh2e_intc_icr_reg_write(sh2e_intc, val);
        break;
    case SH2E_INTC_ISR_REGISTER_ADDRESS_OFFSET:
        sh2e_intc_isr_reg_write(sh2e_intc, val);
        break;
    }
}

/** Dump INTC registers command. */
static bool
dsh2eintc_cmd_dump_intc_regs(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_intc_dump_regs(get_sh2e_intc(dev));
    return true;
}

static bool
dsh2eintc_cmd_dump_configuration(token_t *parm, device_t *const dev)
{
    ASSERT(dev != NULL);

    sh2e_intc_dump_configuration(get_sh2e_intc(dev));
    return true;
}

/*
 * Device commands
 */
static cmd_t dsh2eintc_cmds[] = {
    { "init",
            (fcmd_t) dsh2eintc_cmd_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/intc name" NEXT
                    OPT INT "addr/register block address" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display this help text",
            "Display this help text",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dsh2eintc_cmd_info,
            DEFAULT,
            DEFAULT,
            "Display configuration information",
            "Display configuration information",
            NOCMD },
    { "stat",
            (fcmd_t) dsh2eintc_cmd_stat,
            DEFAULT,
            DEFAULT,
            "Display INTC statistics",
            "Display INTC statistics",
            NOCMD },
    { "rd",
            (fcmd_t) dsh2eintc_cmd_dump_intc_regs,
            DEFAULT,
            DEFAULT,
            "Dump contents of INTC registers",
            "Dump contents of INTC registers",
            NOCMD },
    { "conf",
            (fcmd_t) dsh2eintc_cmd_dump_configuration,
            DEFAULT,
            DEFAULT,
            "Dump INTC configuration",
            "Dump INTC configuration",
            NOCMD },
    { "addintsrc",
            (fcmd_t) dsh2eintc_cmd_add_interrupt_source,
            DEFAULT,
            DEFAULT,
            "Add interrupt source",
            "Add interrupt source <source_id> <priority_pool_index> <priority>",
            REQ INT "source_id" NEXT
                    REQ INT "priority_pool_index" NEXT
                            REQ INT "priority" END },
    LAST_CMD
};

device_type_t dsh2eintc = {
    /* SH-2E INTC is simulated deterministically */
    .nondet = false,

    .name = "dsh2eintc",
    .brief = "Interrupt controller",
    .full = "Interrupt controller device",

    .read16 = dsh2eintc_read16,
    .write16 = dsh2eintc_write16,

    /* Functions */
    .done = dsh2eintc_done,
    /* Commands */
    .cmds = dsh2eintc_cmds
};
