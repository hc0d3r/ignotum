// gcc test/test-ignotum_openmemstr.c lib/ignotum.o -Isrc -o test/bin/test-ignotum_openmemstr


#include <ignotum.h>
#include <err.h>

int main(void){
	ignotum_status status;
	char *target_pid;
	int mem_fd;

	target_pid = "self";
	status = ignotum_openmemstr( target_pid, &mem_fd, O_RDONLY, 0);

	if(status != IGNOTUM_SUCCESS){
		if(status == IGNOTUM_INVALID_PID_STR){
			errx(1,"%s is a invalid pid number", target_pid);
		} else if(status == IGNOTUM_OPEN_MEM_FAILED){
			err(1,"failed to open /proc/%s/mem", target_pid);
		} else if(status == IGNOTUM_PID_T_OVERFLOW){
			err(1,"%s overfloooooooooooooooooooow\n", target_pid);
		}
	} else {
		printf("ignotum_openmem -> %d\n", mem_fd);
		close(mem_fd);
	}

	return 0;

}
