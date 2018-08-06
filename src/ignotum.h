#ifndef _IGNOTUM_H
#define _IGNOTUM_H 1

/* information macros */
#define IGNOTUM_VERSION "0.1"

/* initialization macros */
#define DEFAULT_IGNOTUM_ADDR_RANGE (struct ignotum_addr_range){ 0, 0 }
#define DEFAULT_IGNOTUM_STRING (struct ignotum_string){ NULL, 0 }
#define DEFAULT_IGNOTUM_ADDR_LIST (struct ignotum_addr_list){ DEFAULT_IGNOTUM_ADDR_RANGE, 0, 0, 0, 0, DEFAULT_IGNOTUM_STRING, NULL }
#define DEFAULT_IGNOTUM_MEMSEARCH (struct ignotum_search){ 0, NULL }

#include <stddef.h>
#include <sys/types.h>

typedef struct ignotum_map_list {
	struct ignotum_map_info *map;
	struct ignotum_map_list *next;
} ignotum_map_list_t;

typedef struct ignotum_map_info {
	off_t start_addr;
	off_t end_addr;
	int perms;
	off_t offset;
	dev_t st_dev;
	ino_t st_ino;
	char *pathname;
} ignotum_map_info_t;

typedef struct ignotum_search {
	size_t len;
	off_t *addrs;
} ignotum_search_t;

enum {
	ignotum_read = 1,
	ignotum_exec = 2,
	ignotum_write = 4,
	ignotum_private = 8,
	ignotum_shared = 16
};

ssize_t ignotum_get_map_list(pid_t target_pid, ignotum_map_list_t **out);
ignotum_map_info_t *ignotum_getmapbyaddr(pid_t pid, off_t addr);

ssize_t ignotum_mem_write(pid_t pid, const void *src, size_t n, off_t offset);
ssize_t ignotum_mem_read(pid_t pid, void *out, size_t n, off_t offset);

size_t ignotum_ptrace_write(pid_t pid, const void *data, long addr, size_t n);
size_t ignotum_ptrace_read(pid_t pid, void *output, long addr, size_t n);

ignotum_search_t *ignotum_search(off_t remote_addr, const void *haystack, size_t hlen, const void *needle, size_t nlen);

void free_ignotum_map_list(ignotum_map_list_t **);
void free_ignotum_map_info(ignotum_map_info_t *info);
void free_ignotum_search(ignotum_search_t *);


#endif /* _IGNOTUM_H */
