CC?=gcc
ARCHFLAGS?=-m64
CFLAGS+=-Wall -Wextra -g -O2 $(ARCHFLAGS)

TEST_BIN=bin
IGNOTUM_LIB=../lib/ignotum.o
SRC_DIR=../src/

.PHONY: all clean

SOURCES := $(wildcard *c)
BINS := $(addprefix $(TEST_BIN)/, $(SOURCES:%.c=%))

all: $(BINS)


$(TEST_BIN)/%: %.c
	$(CC) $(CFLAGS) -o $@ $< -I$(SRC_DIR) $(IGNOTUM_LIB)

$(TEST_BIN)/test-ignotum_memread: $(TEST_BIN)/read_my_nops.o test-ignotum_memread.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(SRC_DIR) $(IGNOTUM_LIB)

$(TEST_BIN)/test-ignotum_ptrace_read: $(TEST_BIN)/read_my_nops.o test-ignotum_ptrace_read.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(SRC_DIR) $(IGNOTUM_LIB)

$(TEST_BIN)/read_my_nops.o:
	nasm -f elf64 read_my_nops.asm -o bin/read_my_nops.o


clean:
	rm -f $(BINS) $(TEST_BIN)/read_my_nops.o
