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

#ifndef RISCV_RV64G_CPU_H_
#define RISCV_RV64G_CPU_H_

#define RV64G_REG_COUNT  32

/** Main processor structure */
typedef struct {
	/* Standard registers */
	reg64_t regs[RV64G_REG_COUNT];
	
	/* Program counter */
	ptr64_t pc;
} rv64g_cpu_t;

#endif
