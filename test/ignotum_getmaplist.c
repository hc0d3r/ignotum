#include <ignotum.h>
#include <sys/mman.h>
#include <stdio.h>


int main(void){
    ignotum_maplist_t *addrs = NULL, *i = NULL;

    mmap((void *)0x13370000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0L);

    /* 0 will open /proc/self/maps */
    if( ignotum_getmaplist(0, &addrs) > 0 ){
        for(i=addrs; i!=NULL; i=i->next){
            printf("start-address-> %zx | end-address-> %zx\n", i->map->start_addr, i->map->end_addr);
        }
    }

    free_ignotum_maplist(&addrs);

    return 0;

}
