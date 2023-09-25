/*
 * Copyright (c) 2023 Vojtech Horky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Debug no-memory device. Writing to this area causes simulator to enter
 * interactive mode.
 *
 */

#ifndef DNOMEM_H_
#define DNOMEM_H_

#include "device.h"

extern device_type_t dnomem;

#endif
