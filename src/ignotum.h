#ifndef _IGNOTUM_H
#define _IGNOTUM_H 1

#include <stddef.h>
#include <sys/types.h>

/* globals */
extern const char * const ignotum_version;

/* enum for ignotum_search* */
enum {
    IGNOTUM_NONE,
    IGNOTUM_PARTIAL,
    IGNOTUM_FOUND
};

/* types */

typedef struct ignotum_maplist {
    size_t len;
    struct ignotum_mapinfo *maps;
} ignotum_maplist_t;

typedef struct ignotum_mapinfo {
    off_t start_addr;
    off_t end_addr;
    unsigned char perms[5];
    unsigned char is_x;
    unsigned char is_w;
    unsigned char is_r;
    off_t offset;
    dev_t st_dev;
    ino_t st_ino;
    char *pathname;
} ignotum_mapinfo_t;

typedef struct {
    off_t current;
    off_t match;
    char *search;
    size_t len;
    size_t i;
    int start;
} ignotum_search_t;

/* functions */

ssize_t ignotum_getmaplist(ignotum_maplist_t *list, pid_t pid);
int ignotum_getmapbyaddr(ignotum_mapinfo_t *out, pid_t pid, off_t addr);
int ignotum_getbasemap(ignotum_mapinfo_t *out, pid_t pid, const char *filename, int wildcard);

ssize_t ignotum_mem_write(pid_t pid, const void *buf, size_t n, off_t addr);
ssize_t ignotum_mem_read(pid_t pid, void *buf, size_t n, off_t addr);

ssize_t ignotum_ptrace_write(pid_t pid, const void *buf, size_t n, long addr);
ssize_t ignotum_ptrace_read(pid_t pid, void *buf, size_t n, long addr);

void ignotum_search_init(ignotum_search_t *cs, const void *search, size_t len);
int ignotum_search_loop(ignotum_search_t *cs, off_t *out, off_t vaddr, const void *mem, size_t len);

void free_ignotum_maplist(ignotum_maplist_t *);

#endif /* _IGNOTUM_H */
