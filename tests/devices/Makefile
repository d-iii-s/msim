# SPDX-License-Identifier: Apache-2.0
# Copyright 2019 Charles University

DIFF = diff

-include config.mk

### Phony targets

.PHONY: all clean distclean kernel cstyle check-cstyle check-config



### Default target

all: kernel

kernel: check-config
	$(MAKE) -C kernel

clean:
	$(MAKE) -C kernel clean

distclean:
	$(MAKE) -C kernel distclean
	rm -f config.mk

check-cstyle:
	./tools/check_cstyle.sh kernel/

cstyle:
	find kernel/ -name '*.[ch]' -exec clang-format -style=file -i {} \;


### Usability target

check-config:
    ifeq (,$(wildcard ./config.mk))
	$(error Missing 'config.mk'. Run 'configure.py' to create it)
    endif
