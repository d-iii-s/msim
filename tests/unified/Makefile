
REBUILD_MAKEFILE = rebuild.mk

all: run-tests

.PHONY: all clean rebuild run-tests

run-tests:
	./all.bats

rebuild:
	./discover.py --makefile=$(REBUILD_MAKEFILE) --bats=all.bats
	$(MAKE) -C . -f $(REBUILD_MAKEFILE)

clean:
	@test -f $(REBUILD_MAKEFILE) || echo "Cannot clean without $(REBUILD_MAKEFILE), perhaps you need to run 'make rebuild' first?"
	@test -f $(REBUILD_MAKEFILE) || exit 1
	$(MAKE) -C . -f $(REBUILD_MAKEFILE) clean

