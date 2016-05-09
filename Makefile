CC?=gcc
ARCHFLAGS?=-m64
CFLAGS+=-Wall -Wextra -g -O2 $(ARCHFLAGS)

OBJ=./lib/ignotum.o
SHARED_OBJ=./lib/ignotum.so

INSTALLPROG = /usr/bin/install

SRC_DIR = ./src
TEST_DIR = ./test


.PHONY: all install uninstall test

all: $(OBJ) $(SHARED_OBJ)

$(OBJ): $(SRC_DIR)/ignotum.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I$(SRC_DIR)

$(SHARED_OBJ): $(SRC_DIR)/ignotum.c
	$(CC) -shared -o  $(SHARED_OBJ) $(OBJ) $(CFLAGS)

install: all
	$(INSTALLPROG) $(SHARED_OBJ) /usr/lib64/libignotum.so
	$(INSTALLPROG) ./src/ignotum.h /usr/include/ignotum.h

uninstall:
	-rm -f /lib/libignotum.so /usr/include/ignotum.h

test: all
	$(MAKE) -C $(TEST_DIR)

clean-test:
	$(MAKE) -C $(TEST_DIR) clean

clean:
	rm -f $(OBJ) $(SHARED_OBJ)

clean-all: clean clean-test
