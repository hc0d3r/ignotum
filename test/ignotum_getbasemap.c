#include <stdio.h>
#include <stdlib.h>
#include <ignotum.h>

const char *names[]={
    "*ignotum_getbasemap",
    "*libc*",
    NULL
};

int main(void){
    ignotum_mapinfo_t map;
    const char **ptr;

    for(ptr=names; *ptr; ptr++){
        puts(*ptr);
        if(ignotum_getbasemap(&map, 0, *ptr, 1)){
            printf("map not found using wildcard\n");
        } else {
            printf("map has been found: %s %lx-%lx\n", map.pathname, map.start_addr, map.end_addr);
            free(map.pathname);
        }
    }


    return 0;
}

