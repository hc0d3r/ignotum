#include "ignotum.h"

void ignotum_search_init(ignotum_search_t *cs, const void *search, size_t len){
    cs->search = (char *)search;
    cs->len = len;
    cs->current = 0;
    cs->i = 0;
}

int ignotum_search_loop(ignotum_search_t *cs, off_t *out, off_t vaddr, const void *mem, size_t len){
    const char *aux = mem;
    size_t i = 0;
    int ret;

    if(cs->current != vaddr){
        cs->current = vaddr;
        cs->start = 0;
        cs->i = 0;
    }

    while(i < len && cs->i < cs->len){
        if(aux[i] == cs->search[cs->i]){
            cs->i++, i++;
        } else {
            if(cs->i){
                cs->i = 0;
            } else {
                i++;
            }
        }
    }

    if(cs->i && !cs->start){
        cs->match = cs->current+i-cs->i;
        cs->start = 1;
    } else if(cs->i == 0){
        cs->start = 0;
    }

    if(cs->i == cs->len){
        *out = cs->match;
        cs->i = 0;
        ret = IGNOTUM_FOUND;
    } else if(cs->i){
        ret = IGNOTUM_PARTIAL;
    } else {
        ret = IGNOTUM_NONE;
    }

    cs->current += len;

    return ret;
}
