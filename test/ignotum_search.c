#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

void test2(void){
    ignotum_search_t search;
    ignotum_maplist_t list;
    ignotum_mapinfo_t *map;

    off_t addr;
    size_t i;

    /* alloc two pages next to each other */
    char *ptr = mmap((void *)0xd34d0000, 0x1000, PROT_READ|PROT_WRITE,
        MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, -1, 0);

    /* different permissions to differ the maps,
     * maybe fail in a hardened system */
    char *ptr2 = mmap((void *)0xd34d1000, 0x1000, PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, -1, 0);

    if(ptr == MAP_FAILED || ptr2 == MAP_FAILED){
        printf("failed to mmap()\n");
        return;
    }

    /* make the string avaliable in two differents map */
    memcpy(ptr+0x1000-10, "0123456789abcdef", 16);

    /* 0 = current pid */
    ignotum_getmaplist(&list, 0);

    /* set string to search */
    ignotum_search_init(&search, "0123456789abcdef", 16);

    for(i=0; i<list.len; i++){
        map = list.maps + i;

        /* only read memory that can be read,
         * and isn't associated with a file */
        if(!map->is_r || map->pathname)
            continue;

        printf("searching %lx-%lx\n", map->start_addr, map->end_addr, map->pathname);

        if(ignotum_search_loop(&search, &addr, map->start_addr, (void *)map->start_addr,
            map->end_addr-map->start_addr) == IGNOTUM_FOUND){
            printf("string found at: %lx\n", addr);
        }
    }

    free_ignotum_maplist(&list);
}

int main(void){
    ignotum_search_t search;
    ignotum_mapinfo_t map;

    size_t len, i;
    off_t addr;
    char *aux;

    char test[]="abcdefghijklmnopq ------ leet --------";
    printf("char test[] = %p\n", test);

    /* 0 = current pid */
    ignotum_getmapbyaddr(&map, 0, (off_t)test);
    len = map.end_addr - map.start_addr;
    aux = (char *)map.start_addr;

    ignotum_search_init(&search, "abcdefghijklmnopq ------ leet", 29);
    if(ignotum_search_loop(&search, &addr, map.start_addr, aux, len) != IGNOTUM_FOUND){
        printf("[error] not found ...\n");
    } else {
        printf("string found at addr: %lx | %s\n", addr, (char *)addr);
    }

    /* circular search */
    ignotum_search_init(&search, "------ leet", 11);
    for(i=0; i<len; i+=4){
        if(ignotum_search_loop(&search, &addr, map.start_addr, aux+i, 4) == IGNOTUM_FOUND){
            printf("string found at addr: %lx | %s\n", addr, (char *)addr);
            break;
        }

        /* you must ever update the virtual address or the not will match */
        map.start_addr += 4;
    }

    free(map.pathname);

    printf("\n----- test2 -----\n");
    test2();


    return 0;
}
