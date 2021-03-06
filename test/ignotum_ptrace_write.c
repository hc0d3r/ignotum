#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <wait.h>

static const char change_me[]="You cannot change me !!!";

void test(void){
    printf("(%p) before -> %s\n", change_me, change_me);

    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);

    printf("(%p) after  -> %s\n", change_me, change_me);
}

int main(void){

    pid_t pid = fork();
    if(pid == -1){
        perror("fork()");
    } else if(pid == 0){
        test();
        exit(0);
    }

    waitpid(pid, NULL, 0);
    ssize_t ret = ignotum_ptrace_write(pid, "Yes, I can!", 11, (off_t)change_me);
    ptrace(PTRACE_CONT, pid, 0L, 0L);
    waitpid(pid, NULL, 0);

    printf("bytes written: %zd\n", ret);

    return 0;

}
