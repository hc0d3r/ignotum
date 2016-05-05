// gcc test/test-ignotum_openmap.c lib/ignotum.o -Isrc -o test/bin/test-ignotum_openmap

#include <ignotum.h>
#include <err.h>

int main(void){

    ignotum_status status;
    pid_t target_pid;
    int map_fd;

    target_pid = getpid();
	status = ignotum_openmap( target_pid, &map_fd );

    if(status != IGNOTUM_SUCCESS){
        if(status == IGNOTUM_INVALID_PID_NUMBER){
            errx(1,"%d is a invalid pid number", target_pid);
        } else if(status == IGNOTUM_OPEN_MAP_FAILED){
            err(1,"failed to open /proc/%d/maps", target_pid);
        }
    } else {
        printf("ignotum_openmap -> %d\n", map_fd);
        close(map_fd);
    }

    return 0;
}
