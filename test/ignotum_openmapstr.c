// gcc test/test-ignotum_openmapstr.c lib/ignotum.o -Isrc -o test/bin/test-ignotum_openmapstr

#include <ignotum.h>
#include <err.h>

int main(void){
    ignotum_status status;
    char* target_pid;
    int map_fd;

    target_pid = "self";
	status = ignotum_openmapstr( target_pid, &map_fd );

    if(status != IGNOTUM_SUCCESS){
        if(status == IGNOTUM_INVALID_PID_STR){
            errx(1,"%s is a invalid pid string", target_pid);
        } else if(status == IGNOTUM_OPEN_MAP_FAILED){
            err(1,"failed to open /proc/%s/maps", target_pid);
        }
    } else {
        printf("ignotum_openmapstr -> %d\n", map_fd);
        close(map_fd);
    }

    return 0;
}
