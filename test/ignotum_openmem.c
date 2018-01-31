// gcc test/test-ignotum_openmem.c lib/ignotum.o -Isrc -o test/bin/test-ignotum_openmem


#include <ignotum.h>
#include <err.h>

int main(void){
    ignotum_status status;
    pid_t target_pid;
    int mem_fd;

    target_pid = getpid();
	status = ignotum_openmem( target_pid, &mem_fd, O_RDONLY, 0);

    if(status != IGNOTUM_SUCCESS){
        if(status == IGNOTUM_INVALID_PID_STR){
            errx(1,"%d is a invalid pid number", target_pid);
        } else if(status == IGNOTUM_OPEN_MEM_FAILED){
            err(1,"failed to open /proc/%d/mem", target_pid);
        }
    } else {
        printf("ignotum_openmemstr -> %d\n", mem_fd);
        close(mem_fd);
    }

    return 0;

}
