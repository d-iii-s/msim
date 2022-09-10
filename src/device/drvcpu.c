#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "drvcpu.h"
#include "cpu/general_cpu.h"
#include "cpu/riscv_rv32ima/cpu.h"
#include "cpu/riscv_rv32ima/debug.h"
#include "../main.h"
#include "../assert.h"
#include "../utils.h"
#include "../fault.h"

static bool rv_convert_add_wrapper(void* cpu, ptr64_t virt,  ptr36_t* phys, bool write){
    // use only low 32-bits from virt
    return rv_convert_addr((rv_cpu_t*)cpu, virt.lo, phys, write, false) == rv_exc_none;
}

static const cpu_ops_t rv_cpu = {
	.interrupt_up = (interrupt_func_t)rv_interrupt_up,
	.interrupt_down = (interrupt_func_t)rv_interrupt_down,
	
	.convert_addr = (convert_addr_func_t)rv_convert_add_wrapper,
    
	//.set_pc = (set_pc_func_t)rv_cpu_set_pc, - wrong type cast
	.sc_access = (sc_access_func_t)rv_sc_access
};

static bool drvcpu_init(token_t *parm, device_t *dev){
    
    unsigned int id = get_free_cpuno();

    if(id == MAX_CPUS) {
        error("Maximum CPU count exceeded (%u)", MAX_CPUS);
		return false;
    }

    rv_cpu_t *cpu = safe_malloc_t(rv_cpu_t);
    rv_cpu_init(cpu, id);
    general_cpu_t* gen_cpu = safe_malloc_t(general_cpu_t);
    gen_cpu->cpuno = id;
    gen_cpu->data = cpu;
    gen_cpu->type = &rv_cpu;

    add_cpu(gen_cpu);

    dev->data = gen_cpu;

    return true;
}

static bool drvcpu_info(token_t *parm, device_t *dev){
    printf("RV32IMA\n");
    return true;
}


static bool drvcpu_rd(token_t *parm, device_t *dev){
    ASSERT(dev != NULL);

    rv_reg_dump(get_rv(dev));
    return true;
}

static void drvcpu_done(device_t *dev){
    safe_free(dev->name);
    safe_free(((general_cpu_t *)dev->data)->data);
    safe_free(dev->data)
}

static void drvcpu_step(device_t *dev){
    rv_cpu_step(get_rv(dev));
}



cmd_t drvcpu_cmds[] = {
    {
        "init",
        (fcmd_t) drvcpu_init,
        DEFAULT,
        DEFAULT,
        "Initialization",
        "Initialization",
        REQ STR "pname/processor name" END
    },
    {
        "help",
        (fcmd_t) dev_generic_help,
        DEFAULT,
        DEFAULT,
        "Display this help text",
        "Display this help text",
        OPT STR "cmd/command name" END
    },
    {
        "info",
        (fcmd_t) drvcpu_info,
        DEFAULT,
        DEFAULT,
        "Display configuration information",
        "Display configuration information",
        NOCMD
    },
    {
        "rd",
        (fcmd_t) drvcpu_rd,
        DEFAULT,
        DEFAULT,
        "Dump content of CPU general registers",
        "Dump content of CPU general registers",
        NOCMD
    }
};

device_type_t drvcpu = {
    .nondet = false,

    .name = "drvcpu",

    .brief = "RISC-V RV32IMA processor",

    .full = "RISC-V RV32IMA processor",

    .done = drvcpu_done,
    .step = drvcpu_step,

    .cmds = drvcpu_cmds
};