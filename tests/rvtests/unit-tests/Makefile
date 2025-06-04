# Not to be run on its own, but as src/Makefile rvtest
CC = gcc
CFLAGS = -g -Wall -Wextra
LIBS = -lpcut
SOURCES = $(wildcard *.c)
OBJECTS := $(addsuffix .o,$(basename $(SOURCES)))

# Remove main so we don't cause a linker conflict
MSIM_OBJECTS := $(filter-out main.o, $(MSIM_OBJECTS))

# change relativity of paths
MSIM_OBJECTS := $(addprefix ../../../src/, $(MSIM_OBJECTS))

ARCH32_CFLAGS = $(CFLAGS_BASE) -DARCH=32
ARCH64_CFLAGS = $(CFLAGS_BASE) -DARCH=64
TARGET_32 = tests_32
TARGET_64 = tests_64
OBJECTS_32 := $(addprefix 32bit/, $(addsuffix .o, $(basename $(SOURCES))))
OBJECTS_64 := $(addprefix 64bit/, $(addsuffix .o, $(basename $(SOURCES))))

RM = rm
TARGET = tests

.PHONY: test test32 test64 clean distclean

test: test32 test64

$(TARGET_32): $(OBJECTS_32)
	$(CC) $(ARCH32_CFLAGS) -o $@ $(MSIM_OBJECTS) $(OBJECTS_32) $(LIBS) $(MSIM_LIBS)

32bit/%.o: %.c | 32bit
	$(CC) -c $(ARCH32_CFLAGS) -o $@ $<

test32: $(TARGET_32)
	@echo "Running 32-bit tests..."
	./$(TARGET_32)

$(TARGET_64): $(OBJECTS_64)
		$(CC) $(ARCH64_CFLAGS) -o $@ $(MSIM_OBJECTS) $(OBJECTS_64) $(LIBS) $(MSIM_LIBS)

64bit/%.o: %.c | 64bit
		$(CC) -c $(ARCH64_CFLAGS) -o $@ $<

test64: $(TARGET_64)
	@echo "Running 64-bit tests..."
	./$(TARGET_64)

32bit:
	mkdir -p 32bit

64bit:
	mkdir -p 64bit

clean:
	$(RM) -f $(TARGET_32) $(TARGET_64)
	$(RM) -r 32bit 64bit

distclean: clean
