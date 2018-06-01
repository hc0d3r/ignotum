#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <wait.h>


void read_my_nops(void);

void test(void){
	ptrace(PTRACE_TRACEME);
	kill(getpid(), SIGSTOP);
}

int main(void){
	unsigned char nops[10];
	int i;

	pid_t pid = fork();
	if(pid == -1){
		perror("fork()");
	} else if(pid == 0){
		test();
		exit(0);
	}

	waitpid(pid, NULL, 0);
	size_t ret = ignotum_ptrace_read(pid, nops, (long)read_my_nops, 10);
	ptrace(PTRACE_CONT, pid, 0L, 0L);
	waitpid(pid, NULL, 0);

	printf("bytes read: %zu\n", ret);
	for(i=0; i<10; i++){
		printf("%x ", nops[i]);
	}

	putchar('\n');


	return 0;

}
