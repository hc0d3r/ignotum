#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fnmatch.h>

#include "ignotum.h"
#include "ign_str.c"

static int getnextmap(ignotum_mapinfo_t *map, parser_t *info, int fd){
    int ret = 0;

    if(info->limit > info->i)
        goto do_parser;

    while((info->limit = read(fd, info->buf, sizeof(info->buf))) > 0){
        info->i = 0;
        while(info->limit > info->i){
            do_parser:
            parser(map, info);

            if(info->flag == ignp_end){
                info->flag = ignp_addr_start;
                info->aux_len = 0;

                ret = 1;
                goto end;
            }
        }
    }

    end:
    return ret;
}

static int openmap(pid_t pid){
    char buf[32], *ptr;

    if(pid){
        sprintf(buf, "/proc/%d/maps", pid);
        ptr = buf;
    } else {
        ptr = "/proc/self/maps";
    }

    return open(ptr, O_RDONLY);
}

ssize_t ignotum_getmaplist(ignotum_maplist_t *list, pid_t pid){
    int maps_fd, serr;
    ssize_t ret = -1;

    ignotum_mapinfo_t map;
    parser_t info;

    void *tmp;

    size_t init_alloc = 24;

    list->len = 0;
    list->maps = NULL;

    if((maps_fd = openmap(pid)) == -1){
        goto end;
    }

    list->maps = malloc(init_alloc*sizeof(ignotum_mapinfo_t));
    if(list->maps == NULL)
        goto alloc_error;

    memset(&map, 0, sizeof(ignotum_mapinfo_t));
    info.flag = ignp_addr_start;
    info.aux_len = info.flag = info.limit = info.i = 0;

    while(getnextmap(&map, &info, maps_fd)){
        if(list->len == init_alloc){
            init_alloc += 24;
            tmp = realloc(list->maps, sizeof(ignotum_mapinfo_t)*(init_alloc));
            if(tmp == NULL){
                serr = errno;
                free(map.pathname);
                errno = serr;
                goto alloc_error;
            }
            list->maps = tmp;
        }

        memcpy(list->maps+list->len, &map, sizeof(ignotum_mapinfo_t));
        memset(&map, 0, sizeof(ignotum_mapinfo_t));
        list->len++;
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
    serr = errno;
    close(maps_fd);
    errno = serr;

    end:
    return ret;
}

int ignotum_getmapbyaddr(ignotum_mapinfo_t *out, pid_t pid, off_t addr){
    int maps_fd, serr, ret = 0;
    parser_t info;

    ignotum_mapinfo_t map;

    if((maps_fd = openmap(pid)) == -1){
        goto open_fail;
    }

    memset(&map, 0x0, sizeof(ignotum_mapinfo_t));

    info.flag = ignp_addr_start;
    info.aux_len = info.flag = info.limit = info.i = 0;

    while(getnextmap(&map, &info, maps_fd)){
        if(addr >= map.start_addr && map.end_addr > addr){
            memcpy(out, &map, sizeof(ignotum_mapinfo_t));
            ret = 1;
            break;
        }

        free(map.pathname);
        memset(&map, 0x0, sizeof(ignotum_mapinfo_t));
    }

    serr = errno;
    close(maps_fd);
    errno = serr;

    open_fail:
    return ret;
}

int ignotum_getbasemap(ignotum_mapinfo_t *out, pid_t pid, const char *filename, int wildcard){
    ignotum_mapinfo_t map;
    parser_t info;

    int fd, ret = 1;

    fd = openmap(pid);
    if(fd == -1)
        goto end;

    memset(&map, 0x0, sizeof(ignotum_mapinfo_t));

    info.flag = ignp_addr_start;
    info.aux_len = info.flag = info.limit = info.i = 0;

    while(getnextmap(&map, &info, fd)){
        if(map.pathname && !map.offset){
            if(!wildcard){
                ret = strcmp(map.pathname, filename);
            } else {
                ret = fnmatch(filename, map.pathname, 0);
            }

            if(!ret){
                memcpy(out, &map, sizeof(ignotum_mapinfo_t));
                break;
            }
        }

        free(map.pathname);
        memset(&map, 0x0, sizeof(ignotum_mapinfo_t));
    }

    close(fd);

    end:
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
