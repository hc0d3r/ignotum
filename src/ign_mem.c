#include "ign_mem.h"

ssize_t ignotum_mem_write(pid_t pid, const void *src, size_t n, off_t offset){
    char pathbuf[32], *filename;
    ssize_t ret = -1;

    if(!pid){
        filename = "/proc/self/mem";
    } else {
        filename = pathbuf;
        sprintf(pathbuf, "/proc/%d/mem", pid);
    }

    int fd = open(filename, O_WRONLY);
    if(fd == -1){
        goto end;
    }

    ret = pwrite(fd, src, n, offset);
    close(fd);

    end:
        return ret;
}

ssize_t ignotum_mem_read(pid_t pid, void *out, size_t n, off_t offset){
    char pathbuf[32], *filename;
    ssize_t ret = -1;

    if(!pid){
        filename = "/proc/self/mem";
    } else {
        filename = pathbuf;
        sprintf(pathbuf, "/proc/%d/mem", pid);
    }

    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        goto end;
    }

    ret = pread(fd, out, n, offset);
    close(fd);

    end:
        return ret;
}
