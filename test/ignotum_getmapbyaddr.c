#include <ignotum.h>
#include <stdlib.h>
#include <stdio.h>

int main(void){
    char stack_variable[]="this intead to be in stack\n";
    ignotum_mapinfo_t map;

    if(ignotum_getmapbyaddr(&map, 0, (off_t)stack_variable)){
        printf("stack_variable: %lx, ranges: %lx | %lx, pathname: %s\n", (off_t)stack_variable, map.start_addr, map.end_addr, map.pathname);
        free(map.pathname);
    }

    if(ignotum_getmapbyaddr(&map, 0, (off_t)malloc(0))){
        printf("malloc returns data from %s\n", map.pathname);
        free(map.pathname);
    }

    return 0;
}
