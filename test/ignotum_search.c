#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(void){
    ignotum_mapinfo_t map;
    size_t len, i;
    char *buf;
    off_t addr;

    char test[]="abcdefghijklmnopq ------ leet --------";
    printf("char test[] = %p\n", test);

    /* 0 = current pid */
    ignotum_getmapbyaddr(&map, 0, (off_t)test);
    len = map.end_addr - map.start_addr;
    buf = malloc(len);
    if(buf == NULL){
        perror("malloc()");
        return 1;
    }

    if((size_t)ignotum_mem_read(0, buf, len, map.start_addr) != len){
        perror("ignotum_mem_read()");
        return 1;
    }

    ignotum_search_t search;
    ignotum_search_init(&search, "abcdefghijklmnopq ------ leet", 29);
    if(ignotum_search_loop(&search, &addr, map.start_addr, buf, len) != IGNOTUM_FOUND){
        printf("[error] not found ...\n");
    }

    printf("string found at addr: %lx | %s\n", addr, (char *)addr);

    /* circular search */
    ignotum_search_init(&search, "abcdefghijklmnopq ------ leet", 29);
    for(i=0; i<len; i+=4){
        if(ignotum_search_loop(&search, &addr, map.start_addr, buf, len) == IGNOTUM_FOUND){
            printf("string found at addr: %lx | %s\n", addr, (char *)addr);
            break;
        }

        /* you must ever update the virtual address or the not will match */
        map.start_addr += 4;
    }


    return 0;
}
