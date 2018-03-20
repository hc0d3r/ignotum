#ifndef _IGNOTUM_H
#define _IGNOTUM_H 1

/* information macros */
#define IGNOTUM_VERSION "0.1"

/* initialization macros */
#define DEFAULT_IGNOTUM_ADDR_RANGE (struct ignotum_addr_range){ 0, 0 }
#define DEFAULT_IGNOTUM_STRING (struct ignotum_string){ NULL, 0 }
#define DEFAULT_IGNOTUM_ADDR_LIST (struct ignotum_addr_list){ DEFAULT_IGNOTUM_ADDR_RANGE, 0, 0, 0, 0, DEFAULT_IGNOTUM_STRING, NULL }
#define DEFAULT_IGNOTUM_MEMSEARCH (struct ignotum_mem_search){ 0, NULL }

/* size constant */
#define SIGNED_OVERFLOW_PID_T (pid_t)~((pid_t)1 << ((sizeof(pid_t)*8)-1)) /* 0b100000000000000... */
#define MAX10_PID_T_STR (10*sizeof(pid_t)*CHAR_BIT/33+3) /* note that it dont include minus signal */
#define PATHNAME_LEN 1024
#define wordsize sizeof(long)

/* function macros */
#define check_hex_digit(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define ignotum_free(x) __safefree((void **)&(x))

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef struct ignotum_string {
	char *ptr;
	size_t size;
} ignotum_string_t;

typedef struct ignotum_addr_range {
	off_t start_addr;
	off_t end_addr;
} ignotum_addr_range_t;

typedef struct ignotum_mapped_addr {
	ignotum_addr_range_t range;
	int perms;
	off_t offset;
	dev_t st_dev;
	ino_t st_ino;
	ignotum_string_t pathname;
	struct ignotum_mapped_addr *next;
} ignotum_mapped_addr_t;

typedef struct ignotum_mem_search {
	size_t len;
	off_t *addrs;
} ignotum_mem_search_t;

typedef enum {
	ignotum_first_addr,
	ignotum_second_addr,
	ignotum_flags,
	ignotum_offset,
	ignotum_dev,
	ignotum_inode,
	ignotum_skip_space,
	ignotum_pathname
} ignotum_maps_parser;

enum {
	ignotum_read = 1,
	ignotum_exec = 2,
	ignotum_write = 4,
	ignotum_private = 8,
	ignotum_shared = 16
};

int ignotum_openmem(pid_t pid_number, int mode);

ssize_t ignotum_getmappedaddr(pid_t target_pid, ignotum_mapped_addr_t **out);

int ignotum_mem_write(int mem_fd, off_t offset, const void *src, size_t n);
int ignotum_mem_search(const void *search, size_t search_size, int mem_fd, ignotum_addr_range_t range, ignotum_mem_search_t *out);
int ignotum_mem_read(int mem_fd, off_t offset, void *out, size_t n);

size_t ignotum_ptrace_write(pid_t pid, const void *data, long addr, size_t len);
size_t ignotum_ptrace_read(pid_t pid, void *output, long addr, size_t n);

void free_ignotum_mapped_addr_t(ignotum_mapped_addr_t **);
void free_ignotum_mem_search(ignotum_mem_search_t *);


#endif /* _IGNOTUM_H */
