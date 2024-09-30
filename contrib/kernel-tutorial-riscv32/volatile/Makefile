# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 Charles University

-include config.mk

.PHONY: all clean distclean kernel

all: kernel

kernel:
	$(MAKE) -C kernel

clean:
	$(MAKE) -C kernel clean

distclean:
	$(MAKE) -C kernel distclean
