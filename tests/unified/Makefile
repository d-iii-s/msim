
all: run-tests

.PHONY: all rebuild run-tests

run-tests:
	./all.bats

rebuild:
	./discover.py --makefile=rebuild.mk --bats=all.bats
	$(MAKE) -C . -f rebuild.mk

