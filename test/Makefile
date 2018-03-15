CC?=gcc
CFLAGS+=-Wall -Wextra -g -O2

TEST_BIN=bin
LDFLAGS+=-L../lib/ -lignotum
SRC_DIR=../src/

.PHONY: all clean

SOURCES := $(wildcard *c)
BINS := $(addprefix $(TEST_BIN)/, $(SOURCES:%.c=%))

all: $(BINS)


$(TEST_BIN)/%: %.c
	$(CC) $(CFLAGS) -o $@ $< -I$(SRC_DIR) $(LDFLAGS)

$(TEST_BIN)/ignotum_memread: $(TEST_BIN)/read_my_nops.o ignotum_memread.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(SRC_DIR) $(LDFLAGS)

$(TEST_BIN)/ignotum_ptrace_memread: $(TEST_BIN)/read_my_nops.o ignotum_ptrace_memread.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(SRC_DIR) $(LDFLAGS)

$(TEST_BIN)/read_my_nops.o:
	nasm -f elf64 read_my_nops.asm -o bin/read_my_nops.o


clean:
	rm -f $(BINS) $(TEST_BIN)/read_my_nops.o
