#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "drvcpu.h"
#include "cpu/general_cpu.h"
#include "../main.h"
#include "../utils.h"
#include "../fault.h"

//TODO: General CPU

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
    gen_cpu->type = NULL;

    add_cpu(gen_cpu);

    dev->data = gen_cpu;

    return true;
}

static bool drvcpu_info(token_t *parm, device_t *dev){
    printf("RV32IMA\n");
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
    }
};

device_type_t drvcpu = {
    .nondet = false,

    .name = "drvcpu",

    .brief = "RISC-V RV32I(MA) processor",

    .full = "RISC-V RV32I(MA) processor",

    .done = drvcpu_done,
    .step = drvcpu_step,

    .cmds = drvcpu_cmds
};