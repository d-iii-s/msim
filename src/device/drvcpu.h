#ifndef DRVCPU
#define DRVCPU

#include "device.h"
#include "cpu/general_cpu.h"
#include "cpu/riscv_rv32ima/cpu.h"

#define get_rv(dev) (rv32ima_cpu_t *)(((general_cpu_t *)dev->data)->data)

extern device_type_t drvcpu;

#endif // DRVCPU