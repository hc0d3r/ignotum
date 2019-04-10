#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void read_my_nops(void);

void test(void){
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    _exit(0);
}

int main(void){
    char buf[0x2000];
    size_t i;
    char c;

    void *ptr = mmap((void *)0xcafe0000, 0x1000, PROT_READ|PROT_WRITE,
        MAP_SHARED|MAP_ANONYMOUS|MAP_FIXED, -1, 0);
    if(ptr == MAP_FAILED){
        perror("mmap()");
        return 1;
    }

    for(c=0, i=0; i<0x1000; i++, c++){
        *(char *)(ptr+i) = c;
    }

    pid_t pid = fork();
    if(pid == 0){
        test();
        exit(0);
    }

    wait(NULL);

    for(i=0; i<0x1000; i++){
        ssize_t ret = ignotum_ptrace_read(pid, buf, 0x2000, (long)ptr);
        if(ret == -1 || memcmp(buf, (void *)ptr, (size_t)ret)){
            printf("error at iter: %zd | addr = %p | ret = %zd\n", i, ptr, ret);
            break;
        }
        (char *)ptr++;
    }

    if(ignotum_ptrace_read(pid, buf, 0, 0)){
        printf("error, must return 0");
    }

    ssize_t ret = ignotum_ptrace_read(pid, buf, 10, (off_t)read_my_nops);
    printf("bytes read: %zd\n", ret);
    for(i=0; i<10; i++){
        printf("%x ", (unsigned char)buf[i]);
    }
    putchar('\n');

    ptrace(PTRACE_DETACH, pid, 0, SIGCONT);

    return 0;
}
