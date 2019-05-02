CC?=gcc
CFLAGS+=-Wall -Wextra -O2

OBJDIR = lib

OBJS =		$(OBJDIR)/ign_mem.o \
			$(OBJDIR)/ign_ptrace.o \
			$(OBJDIR)/ign_str.o \
			$(OBJDIR)/ign_maps.o \
			$(OBJDIR)/ign_search.o

#OBJ=./lib/ignotum.o
SHARED_OBJ=./lib/libignotum.so
STATIC_OBJ=./lib/libignotum.a

INSTALLPROG=/usr/bin/install
INSTALL_LIB_DIR?=/usr/lib64
INSTALL_HEADER_DIR?=/usr/include

DOC_DIR=doc
MAN_PAGES=$(shell find $(DOC_DIR) -type f -name '*.3')
GZIP_PAGES=$(MAN_PAGES:%=%.gz)
MAN_PAGES_DIR=/usr/share/man


SRC_DIR = src
TEST_DIR = test


.PHONY: all install uninstall test

all: $(SHARED_OBJ) $(STATIC_OBJ)

#$(OBJ): $(SRC_DIR)/ignotum.c
$(OBJDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -c -o $@ $< -I.

$(STATIC_OBJ): $(OBJS)
	ar -cvr $(STATIC_OBJ) $(OBJS)

$(SHARED_OBJ): $(OBJS)
	$(CC) -shared -o  $(SHARED_OBJ) $(OBJS) $(CFLAGS)

install: all
	$(INSTALLPROG) $(SHARED_OBJ) $(INSTALL_LIB_DIR)/libignotum.so
	$(INSTALLPROG) $(STATIC_OBJ) $(INSTALL_LIB_DIR)/libignotum.a
	$(INSTALLPROG) src/ignotum.h $(INSTALL_HEADER_DIR)/ignotum.h

uninstall:
	-rm -f $(INSTALL_LIB_DIR)/libignotum.so $(INSTALL_LIB_DIR)/libignotum.a $(INSTALL_HEADER_DIR)/ignotum.h

test: all
	$(MAKE) -C $(TEST_DIR)

clean-test:
	$(MAKE) -C $(TEST_DIR) clean

clean:
	rm -f $(OBJS) $(SHARED_OBJ) $(STATIC_OBJ)

clean-all: clean clean-test clean-doc

doc: $(GZIP_PAGES)

$(DOC_DIR)/%.3.gz: $(DOC_DIR)/%.3
	gzip -c $^ > $@

doc-install: doc
	$(INSTALLPROG) -t $(MAN_PAGES_DIR)/man3/ $(GZIP_PAGES)

clean-doc:
	rm -f $(GZIP_PAGES)
