override CFLAGS+=-Wall -Wextra -O2

OBJDIR=lib
OBJS =		$(OBJDIR)/ign_mem.o \
		$(OBJDIR)/ign_ptrace.o \
		$(OBJDIR)/ign_maps.o \
		$(OBJDIR)/ign_search.o

VERSION=0.1

SHARED_OBJ=libignotum.so.$(VERSION)
STATIC_OBJ=libignotum.a

INSTALLPROG=/usr/bin/install
PREFIX?=/usr

DOC_DIR=doc
MAN_PAGES=$(shell find $(DOC_DIR) -type f -name '*.3')
GZIP_PAGES=$(MAN_PAGES:%=%.gz)
MAN_PAGES_DIR=$(PREFIX)/share/man

SRC_DIR = src
TEST_DIR = test

.PHONY: all install uninstall test

all: lib/$(SHARED_OBJ) lib/$(STATIC_OBJ) lib/libignotum.so

$(OBJDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I.

lib/$(STATIC_OBJ): $(OBJS)
	ar -cvr $@ $^

lib/libignotum.so: lib/$(SHARED_OBJ)
	ln -sf $(SHARED_OBJ) $@

lib/$(SHARED_OBJ): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(SHARED_OBJ) -o lib/$(SHARED_OBJ) $(OBJS)

install: all
	$(INSTALLPROG) lib/$(SHARED_OBJ) $(PREFIX)/lib
	$(INSTALLPROG) lib/$(STATIC_OBJ) $(PREFIX)/lib
	$(INSTALLPROG) lib/libignotum.so $(PREFIX)/lib
	$(INSTALLPROG) src/ignotum.h $(PREFIX)/include/ignotum.h

uninstall:
	-rm -f $(PREFIX)/lib/$(SHARED_OBJ) $(PREFIX)/lib/libignotum.so \
	$(PREFIX)/lib/libignotum.a $(PREFIX)/include/ignotum.h

test: all
	$(MAKE) -C $(TEST_DIR)

clean-test:
	$(MAKE) -C $(TEST_DIR) clean

clean:
	rm -f $(OBJS) lib/$(STATIC_OBJ) lib/libignotum.so*

clean-all: clean clean-test clean-doc

doc: $(GZIP_PAGES)

$(DOC_DIR)/%.3.gz: $(DOC_DIR)/%.3
	gzip -c $^ > $@

doc-install: doc
	$(INSTALLPROG) -t $(MAN_PAGES_DIR)/man3/ $(GZIP_PAGES)

clean-doc:
	rm -f $(GZIP_PAGES)
