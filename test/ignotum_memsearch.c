#include <ignotum.h>

int main(void){
    pid_t target_pid;
    int map_fd, mem_fd;
    size_t j;

    char test[]="abcdefghijklmnopq ------ leet --------";
    (void)test;

    ignotum_mapped_addr_t *addrs = NULL, *i = NULL;
    ignotum_mem_search_t result = DEFAULT_IGNOTUM_MEMSEARCH;

    target_pid = getpid();
	ignotum_openmap( target_pid, &map_fd );

    printf("char test[] = %p\n", test);

    if( ignotum_getmappedaddr(map_fd, &addrs) ){
        for(i=addrs; i!=NULL; i=i->next){
            if(i->pathname.ptr == NULL)
                continue;

            if(!strcmp("[stack]", i->pathname.ptr)){
                ignotum_openmem(target_pid, &mem_fd, O_RDONLY, 0);
                ignotum_memsearch("leet", 4, mem_fd , i->range, &result);

                if(result.len){
                    for(j=0; j<result.len; j++){
                        printf("leet found at =: %zx | %s\n", result.addrs[j], (char *)result.addrs[j] );
                    }
                }

                close(mem_fd);
                break;
            }

        }
    }

    free_ignotum_mapped_addr_t(&addrs);
    free_ignotum_mem_search(&result);
    close(map_fd);

    return 0;

}
