# Ignotum

a simple lib to help read and write in mapped memory regions of a linux process

### Functions:

```c
int ignotum_openmem(pid_t pid_number, int mode);

size_t ignotum_get_map_list(pid_t target_pid, ignotum_map_list_t **out);
ignotum_map_info_t *ignotum_getmapbyaddr(pid_t pid, off_t addr);

ssize_t ignotum_mem_write(int mem_fd, const void *src, size_t n, off_t offset);
ssize_t ignotum_mem_read(int mem_fd, void *out, size_t n, off_t offset);
int ignotum_mem_search(int mem_fd, const void *search, size_t search_size, ignotum_addr_range_t range, ignotum_mem_search_t *out);


size_t ignotum_ptrace_write(pid_t pid, const void *data, size_t len, long addr);
size_t ignotum_ptrace_read(pid_t pid, void *output, size_t n, long addr);


void free_ignotum_map_list(ignotum_mapped_addr_t **);
void free_ignotum_map_info(ignotum_map_info_t *info);
void free_ignotum_mem_search(ignotum_mem_search_t *);

```

### Types:

```c
typedef struct ignotum_string {
    char *ptr;
    size_t size;
} ignotum_string_t;

typedef struct ignotum_addr_range {
    off_t start_addr;
    off_t end_addr;
} ignotum_addr_range_t;

typedef struct ignotum_map_list {
	struct ignotum_map_info *map;
	struct ignotum_map_list *next;
} ignotum_map_list_t;

typedef struct ignotum_map_info {
	ignotum_addr_range_t range;
	int perms;
	off_t offset;
	dev_t st_dev;
	ino_t st_ino;
	ignotum_string_t pathname;
} ignotum_map_info_t;

typedef struct ignotum_mem_search {
    size_t len;
    off_t *addrs;
} ignotum_mem_search_t;

```

### Compiling:

```
make # create ignotum.o and libignotum.so
make install # install the lib
make test # optional, compile the tests
```

#### Options:

```
CC - compiler (Default: gcc)
INSTALL_LIB_DIR - dir to install .so file (Default: /usr/lib64)
INSTALL_HEADER_DIR - dir to install .h file (Default: /usr/include)
```

### Documentation:

see the man files at doc/
