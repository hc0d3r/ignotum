#include <ignotum.h>

int main(void){
    pid_t target_pid;
    int map_fd;

    ignotum_mapped_addr_t *addrs = NULL, *i = NULL;

    target_pid = getpid();
	ignotum_openmap( target_pid, &map_fd );

    if( ignotum_getmappedaddr(map_fd, &addrs) ){
        for(i=addrs; i!=NULL; i=i->next){
            printf("start-address-> %zx | end-address-> %zx\n", i->range.start_addr, i->range.end_addr);
        }
    }

    free_ignotum_mapped_addr_t(&addrs);
    close(map_fd);

    return 0;

}
