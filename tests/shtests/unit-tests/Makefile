# Not to be run on its own, but as src/Makefile shtest
CC = gcc
CFLAGS = -g -Wall -Wextra
LIBS = -lpcut
SOURCES = $(wildcard *.c)
OBJECTS := $(addsuffix .o,$(basename $(SOURCES)))

# Remove main so we don't cause a linker conflict
MSIM_OBJECTS := $(filter-out main.o, $(MSIM_OBJECTS))

# change relativity of paths
MSIM_OBJECTS := $(addprefix ../../../src/, $(MSIM_OBJECTS))

RM = rm
TARGET = tests

.PHONY: test clean distclean

test: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MSIM_OBJECTS) $(OBJECTS) $(LIBS) $(MSIM_LIBS)
	./$(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) -f $(TARGET) $(OBJECTS)

distclean: clean
