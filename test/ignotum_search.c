#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(void){
    size_t j, ret, i;

    char test[]="abcdefghijklmnopq ------ leet --------";
    (void)test;

    ignotum_maplist_t addrs = {.len = 0, .maps = NULL};
    ignotum_search_t result = DEFAULT_IGNOTUM_MEMSEARCH;

    printf("char test[] = %p\n", test);

    if(ignotum_getmaplist(&addrs, 0) > 0){
        for(i=0; i<addrs.len; i++){
            if(addrs.maps[i].pathname == NULL)
                continue;

            if(!strcmp("[stack]", addrs.maps[i].pathname)){
                size_t len = addrs.maps[i].end_addr-addrs.maps[i].start_addr;
                char *data = malloc(len);

                len = ignotum_mem_read(0, data, len, addrs.maps[i].start_addr);
                ret = ignotum_search(&result, addrs.maps[i].start_addr, data, len, "leet", 4);

                if(ret){
                    for(j=0; j<result.len; j++){
                        printf("leet found at =: %zx | %s\n", result.addrs[j], (char *)result.addrs[j] );
                    }

                    free_ignotum_search(&result);
                }

                break;
            }
        }
    }

    free_ignotum_maplist(&addrs);
    return 0;

}
