override CFLAGS+=-Wall -Wextra -O2

OBJDIR=lib
OBJS =		$(OBJDIR)/ign_mem.o \
		$(OBJDIR)/ign_ptrace.o \
		$(OBJDIR)/ign_maps.o \
		$(OBJDIR)/ign_search.o

SHARED_OBJ=lib/libignotum.so.0.1
STATIC_OBJ=lib/libignotum.a

INSTALLPROG=/usr/bin/install
PREFIX?=/usr

DOC_DIR=doc
MAN_PAGES=$(shell find $(DOC_DIR) -type f -name '*.3')
GZIP_PAGES=$(MAN_PAGES:%=%.gz)
MAN_PAGES_DIR=$(PREFIX)/share/man

SRC_DIR = src
TEST_DIR = test

.PHONY: all install uninstall test

all: $(SHARED_OBJ) lib/libignotum.so $(STATIC_OBJ)

$(OBJDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I.

$(STATIC_OBJ): $(OBJS)
	ar -cvr $(STATIC_OBJ) $(OBJS)

lib/libignotum.so: $(SHARED_OBJ)
	ln -sf libignotum.so.0.1 lib/libignotum.so

$(SHARED_OBJ): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,libignotum.so.0.1 -o $(SHARED_OBJ) $(OBJS)

install: all
	$(INSTALLPROG) $(SHARED_OBJ) $(PREFIX)/lib
	$(INSTALLPROG) $(STATIC_OBJ) $(PREFIX)/lib
	$(INSTALLPROG) lib/libignotum.so $(PREFIX)/lib
	$(INSTALLPROG) src/ignotum.h $(PREFIX)/include/ignotum.h

uninstall:
	-rm -f $(PREFIX)/lib/libignotum.so \
	$(PREFIX)/lib/libignotum.a $(PREFIX)/include/ignotum.h

test: all
	$(MAKE) -C $(TEST_DIR)

clean-test:
	$(MAKE) -C $(TEST_DIR) clean

clean:
	rm -f $(OBJS) $(SHARED_OBJ) $(STATIC_OBJ) lib/libignotum.so

clean-all: clean clean-test clean-doc

doc: $(GZIP_PAGES)

$(DOC_DIR)/%.3.gz: $(DOC_DIR)/%.3
	gzip -c $^ > $@

doc-install: doc
	$(INSTALLPROG) -t $(MAN_PAGES_DIR)/man3/ $(GZIP_PAGES)

clean-doc:
	rm -f $(GZIP_PAGES)
