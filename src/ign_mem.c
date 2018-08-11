#include "ign_mem.h"

ssize_t ignotum_mem_write(pid_t pid, const void *buf, size_t n, off_t addr){
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

    ret = pwrite(fd, buf, n, addr);
    close(fd);

    end:
        return ret;
}

ssize_t ignotum_mem_read(pid_t pid, void *buf, size_t n, off_t addr){
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

    ret = pread(fd, buf, n, addr);
    close(fd);

    end:
        return ret;
}
