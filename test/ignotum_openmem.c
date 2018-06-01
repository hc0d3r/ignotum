// gcc test/test-ignotum_openmem.c lib/ignotum.o -Isrc -o test/bin/test-ignotum_openmem


#include <ignotum.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>

int main(void){
	pid_t target_pid;
	int mem_fd;

	target_pid = getpid();
	mem_fd = ignotum_openmem( target_pid, O_RDONLY);

	if(mem_fd == -1){
		err(1,"failed to open /proc/%d/mem", target_pid);
	} else {
		printf("ignotum_openmemstr -> %d\n", mem_fd);
		close(mem_fd);
	}

	return 0;

}
