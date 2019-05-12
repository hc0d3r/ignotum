override CFLAGS+=-Wall -Wextra -O2

OBJDIR=lib
OBJS =		$(OBJDIR)/ign_mem.o \
		$(OBJDIR)/ign_ptrace.o \
		$(OBJDIR)/ign_maps.o \
		$(OBJDIR)/ign_search.o

STATIC_OBJ=libignotum.a
SRC_DIR = src

lib/$(STATIC_OBJ): $(OBJS)
	ar -cvr $@ $^

$(OBJDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $< -I.

clean:
	rm -f $(OBJS) lib/$(STATIC_OBJ)
