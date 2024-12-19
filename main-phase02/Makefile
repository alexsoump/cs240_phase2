# CC = gcc-10 # Compiler version installed in most department machines
# CC = g++-10 # For C++ compilation
CC = gcc
CFLAGS = -g -O0 -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter  #-DDEBUG_PRINTS_ENABLED #-Wno-unused-parameter
# Uncomment -DDEBUG_PRINTS_ENABLED to enable event parsing debug prints

LDLIBS = -lm
SRC = main.c voting.c
INCLUDE = voting.h
TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(INCLUDE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)
