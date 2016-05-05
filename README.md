# Ignotum

a simple lib to help read and write in mapped memory regions of process on linux

### Functions:

```c
ignotum_status ignotum_openmap(pid_t pid_number, int *fd_out);
ignotum_status ignotum_openmapstr(const char *pid_str, int *fd_out);
ignotum_status ignotum_openmem(pid_t pid_number, int *fd_out, int mode, int attach_pid);
ignotum_status ignotum_openmemstr(const char *pid_str, int *fd_out, int mode, int attach_pid);

size_t ignotum_getmappedaddr(int maps_fd, ignotum_mapped_addr_t **out);

int ignotum_memwrite(int mem_fd, off_t offset, const void *src, size_t n);
int ignotum_memsearch(const void *search, size_t search_size, int mem_fd, ignotum_addr_range_t range, ignotum_mem_search_t *out);
int ignotum_memread(int mem_fd, off_t offset, void *out, size_t n);

void free_ignotum_mapped_addr_t(ignotum_mapped_addr_t **);
void free_ignotum_mem_search(ignotum_mem_search_t *);

```
