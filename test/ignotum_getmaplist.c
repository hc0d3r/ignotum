#include <ignotum.h>
#include <sys/mman.h>
#include <stdio.h>


int main(void){
    ignotum_maplist_t addrs;
    size_t i = 0;
    ssize_t nb;

    mmap((void *)0x13370000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0L);

    /* 0 will open /proc/self/maps */
    if((nb = ignotum_getmaplist(16343, &addrs)) > 0){
        printf("maps total --> %zd\n", nb);
        for(i=0; i<addrs.len; i++){
            printf("start-address-> %zx | end-address-> %zx\n", addrs.maps[i].start_addr, addrs.maps[i].end_addr);
        }
    }

    free_ignotum_maplist(&addrs);

    return 0;

}
