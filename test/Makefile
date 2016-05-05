CC?=gcc
ARCHFLAGS?=-m64
CFLAGS+=-Wall -Wextra -g -O2 $(ARCHFLAGS)

TEST_BIN = ./bin
IGNOTUM_LIB=../lib/ignotum.o
SRC_DIR=../src/

.PHONY: all clean

BIN_TEST = $(TEST_BIN)/test-ignotum_openmap \
$(TEST_BIN)/test-ignotum_openmem \
$(TEST_BIN)/test-ignotum_openmapstr \
$(TEST_BIN)/test-ignotum_openmemstr \
$(TEST_BIN)/test-ignotum_getmappedaddr \
$(TEST_BIN)/test-ignotum_memwrite \
$(TEST_BIN)/test-ignotum_memsearch

all: $(BIN_TEST) $(TEST_BIN)/test-ignotum_memread

$(TEST_BIN)/%: %.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(SRC_DIR) $(IGNOTUM_LIB)

$(TEST_BIN)/test-ignotum_memread: $(TEST_BIN)/read_my_nops.o
	gcc test-ignotum_memread.c -o bin/test-ignotum_memread $(TEST_BIN)/read_my_nops.o $(IGNOTUM_LIB) $(CFLAGS) -I$(SRC_DIR)

$(TEST_BIN)/read_my_nops.o:
	nasm -f elf64 read_my_nops.asm -o bin/read_my_nops.o

clean:
	rm -f $(BIN_TEST) $(TEST_BIN)/test-ignotum_memread $(TEST_BIN)/read_my_nops.o
