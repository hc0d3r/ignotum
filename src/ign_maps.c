#include "ign_maps.h"

ssize_t ignotum_getmaplist(pid_t target_pid, ignotum_maplist_t **out){
    int maps_fd, flag, i, size, aux_len;
    ignotum_mapinfo_t *info;
    char buf[1024];

    ssize_t ret = -1;

    if(target_pid)
        sprintf(buf, "/proc/%d/maps", target_pid);
    else
        memcpy(buf, "/proc/self/maps", 16);

    if((maps_fd = open(buf, O_RDONLY)) == -1){
        goto end;
    }

    info = calloc(1, sizeof(ignotum_mapinfo_t));
    flag = ignp_addr_start;
    aux_len = 0;
    ret = 0;

    while((size = read(maps_fd, buf, sizeof(buf))) > 0 ){
        for(i=0; i<size;){
            parser(info, buf, &i, size, &flag, &aux_len);
            if(flag == ignp_end){
                *out = malloc(sizeof(ignotum_maplist_t));
                (*out)->map = info;
                (*out)->next = NULL;
                out = &((*out)->next);

                info = calloc(1, sizeof(ignotum_mapinfo_t));
                flag = ignp_addr_start;
                aux_len = 0;
                ret++;
            }
        }
    }

    free(info);

    close(maps_fd);

    end:
        return ret;
}

int ignotum_getmapbyaddr(ignotum_mapinfo_t *out, pid_t pid, off_t addr){
    int maps_fd, flag, i, size, aux_len, ret = 1;
    ignotum_mapinfo_t tmp;
    char buf[1024];

    if(pid)
        sprintf(buf, "/proc/%d/maps", pid);
    else
        memcpy(buf, "/proc/self/maps", 16);

    if((maps_fd = open(buf, O_RDONLY)) == -1){
        goto open_fail;
    }

    memset(&tmp, 0x0, sizeof(ignotum_mapinfo_t));

    flag = ignp_addr_start;
    aux_len = 0;

    while((size = read(maps_fd, buf, sizeof(buf))) > 0){
        for(i=0; i<size;){
            parser(&tmp, buf, &i, size, &flag, &aux_len);
            if(flag == ignp_end){
                if(tmp.start_addr <= addr && addr <= tmp.end_addr){
                    memcpy(out, &tmp, sizeof(ignotum_mapinfo_t));
                    ret = 1;
                    goto end;
                }

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

void free_ignotum_maplist(ignotum_maplist_t **addr){
    ignotum_maplist_t *aux;

    while(*addr){
        aux = (*addr)->next;
        free_ignotum_mapinfo((*addr)->map);
        free(*addr);
        *addr = aux;
    }

    addr = NULL;

}

void free_ignotum_mapinfo(ignotum_mapinfo_t *info){
    free(info->pathname);
    free(info);
}
