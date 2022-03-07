/*
 * Copyright (c) 2018 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV64G simulation
 *
 */

#ifndef RISCV_RV32IMA_CPU_H_
#define RISCV_RV32IMA_CPU_H_


#include <stdint.h>

/** Main processor structure */
typedef struct {
	uint32_t pc;
} rv32ima_cpu_t;

#endif //RISCV_RV32IMA_CPU_H_
