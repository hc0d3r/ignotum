#include <ignotum.h>
#include <sys/mman.h>


int main(void){
    pid_t target_pid;
    int map_fd;

    ignotum_mapped_addr_t *addrs = NULL, *i = NULL;

	mmap((void *)0x13370000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0L);

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
