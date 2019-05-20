#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ignotum.h"
#include "ign_str.c"

ssize_t ignotum_getmaplist(ignotum_maplist_t *list, pid_t pid){
    int maps_fd, flag, i, size, aux_len;
    ignotum_mapinfo_t info;
    ssize_t ret = -1;
    char buf[1024], *ptr;
    void *tmp;

    size_t init_alloc = 24;

    list->len = 0;
    list->maps = NULL;

    if(pid){
        sprintf(buf, "/proc/%d/maps", pid);
        ptr = buf;
    } else {
        ptr = "/proc/self/maps";
    }

    if((maps_fd = open(ptr, O_RDONLY)) == -1){
        goto end;
    }

    list->maps = malloc(init_alloc*sizeof(ignotum_mapinfo_t));
    if(list->maps == NULL)
        goto alloc_error;

    memset(&info, 0, sizeof(ignotum_mapinfo_t));
    flag = ignp_addr_start;
    aux_len = 0;

    while((size = read(maps_fd, buf, sizeof(buf))) > 0){
        for(i=0; i<size;){
            parser(&info, buf, &i, size, &flag, &aux_len);
            if(flag == ignp_end){
                if(list->len == init_alloc){
                    init_alloc += 24;
                    tmp = realloc(list->maps, sizeof(ignotum_mapinfo_t)*(init_alloc));
                    if(tmp == NULL){
                        free(info.pathname);
                        goto alloc_error;
                    }
                    list->maps = tmp;
                }

                memcpy(&(list->maps[list->len]), &info, sizeof(ignotum_mapinfo_t));
                memset(&info, 0, sizeof(ignotum_mapinfo_t));
                list->len++;

                flag = ignp_addr_start;
                aux_len = 0;
            }
        }
    }

    if(list->len && (list->len < init_alloc)){
        // try dealloc memory, if fails ignore
        tmp = realloc(list->maps, sizeof(ignotum_mapinfo_t)*(list->len));
        if(tmp != NULL){
            list->maps = tmp;
        }
    }

    ret = list->len;

    alloc_error:
        close(maps_fd);

    end:
        return ret;
}

int ignotum_getmapbyaddr(ignotum_mapinfo_t *out, pid_t pid, off_t addr){
    int maps_fd, flag, i, size, aux_len, ret = 0;
    ignotum_mapinfo_t tmp;
    char buf[1024], *ptr;

    if(pid){
        sprintf(buf, "/proc/%d/maps", pid);
        ptr = buf;
    } else {
        ptr = "/proc/self/maps";
    }

    if((maps_fd = open(ptr, O_RDONLY)) == -1){
        goto open_fail;
    }

    memset(&tmp, 0x0, sizeof(ignotum_mapinfo_t));

    flag = ignp_addr_start;
    aux_len = 0;

    while((size = read(maps_fd, buf, sizeof(buf))) > 0){
        for(i=0; i<size;){
            parser(&tmp, buf, &i, size, &flag, &aux_len);
            if(flag == ignp_end){
                if(addr >= tmp.start_addr && tmp.end_addr > addr){
                    memcpy(out, &tmp, sizeof(ignotum_mapinfo_t));
                    ret = 1;
                    goto end;
                }

                free(tmp.pathname);
                memset(&tmp, 0x0, sizeof(ignotum_mapinfo_t));
                flag = ignp_addr_start;
                aux_len = 0;
            }
        }
    }


    end:
        close(maps_fd);
    open_fail:
        return ret;
}

void free_ignotum_maplist(ignotum_maplist_t *list){
    size_t i;
    for(i=0; i<list->len; i++){
        free(list->maps[i].pathname);
    }

    free(list->maps);
    list->maps = NULL;
    list->len = 0;
}
