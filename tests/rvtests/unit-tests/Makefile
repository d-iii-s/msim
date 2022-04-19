CC = gcc
CFLAGS = -g -Wall -Wextra
LIBS = -lpcut
SOURCES = $(wildcard *.c)
RM = rm
TARGET = tests

.PHONY: test clean distclean

test:
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	./$(TARGET)

clean:
	$(RM) -f $(TARGET)

distclean: clean
