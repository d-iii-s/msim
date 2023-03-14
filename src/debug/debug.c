/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Useful debugging features
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "debug.h"
#include "../device/cpu/mips_r4000/cpu.h"
#include "../device/cpu/riscv_rv32ima/cpu.h"
#include "../device/dr4kcpu.h"
#include "../device/mem.h"
#include "../assert.h"
#include "../env.h"
#include "../main.h"
#include "../utils.h"

void dbg_print_device_info(device_t *dev)
{
	printf("%-10s %-10s ", dev->name, dev->type->name);
	// FIXME cmd_run_by_name("info", &pars_end, dev->type->cmds, dev);
}

/** Show statistics for specified device
 *
 */
void dbg_print_device_stat(device_t *dev)
{
	printf("%-10s %-10s ", dev->name, dev->type->name);
	// FIXME cmd_run_by_name("stat", &pars_end, dev->type->cmds, dev);
}

void dbg_print_devices(device_filter_t filter)
{
	printf("[  name  ] [  type  ] [ parameters...\n");

	device_t *device = NULL;
	bool device_found = false;
	token_t token_end[] = {
		{ .ttype = tt_end }
	};

	while (dev_next(&device, filter)) {
		device_found = true;
		printf("%-10s %-10s ", device->name, device->type->name);
		cmd_run_by_name("info", token_end, device->type->cmds, device);
	}

	if (!device_found) {
		printf("No matching devices found.\n");
	}
}

void dbg_print_devices_stat(device_filter_t filter)
{
	// FIXME
}
