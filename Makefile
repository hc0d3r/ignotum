CC?=gcc
ARCHFLAGS?=-m64
CFLAGS+=-Wall -Wextra -O2 $(ARCHFLAGS)

OBJ=./lib/ignotum.o
SHARED_OBJ=./lib/ignotum.so

INSTALLPROG = /usr/bin/install
INSTALL_LIB_DIR?=/usr/lib64
INSTALL_HEADER_DIR?=/usr/include

SRC_DIR = ./src
TEST_DIR = ./test


.PHONY: all install uninstall test

all: $(OBJ) $(SHARED_OBJ)

$(OBJ): $(SRC_DIR)/ignotum.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I$(SRC_DIR)

$(SHARED_OBJ): $(SRC_DIR)/ignotum.c
	$(CC) -shared -o  $(SHARED_OBJ) $(OBJ) $(CFLAGS)

install: all
	$(INSTALLPROG) $(SHARED_OBJ) $(INSTALL_LIB_DIR)/libignotum.so
	$(INSTALLPROG) ./src/ignotum.h $(INSTALL_HEADER_DIR)/ignotum.h

uninstall:
	-rm -f $(INSTALL_LIB_DIR)/libignotum.so $(INSTALL_HEADER_DIR)/ignotum.h

test: all
	$(MAKE) -C $(TEST_DIR)

clean-test:
	$(MAKE) -C $(TEST_DIR) clean

clean:
	rm -f $(OBJ) $(SHARED_OBJ)

clean-all: clean clean-test
