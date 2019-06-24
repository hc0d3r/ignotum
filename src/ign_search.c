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

    if(cs->current+cs->i != (size_t)vaddr){
        cs->i = 0;
        cs->current = vaddr;
    }

    while(i < len && cs->i < cs->len){
        if(aux[i] == cs->search[cs->i]){
            cs->i++;
            i++;
        } else {
            if(cs->i){
                cs->i = 0;
            } else {
                i++;
            }

            cs->current = vaddr+i;
        }
    }

    if(cs->i == cs->len){
        *out = cs->current;
        cs->i = 0;
        ret = IGNOTUM_FOUND;
    } else if(cs->i){
        ret = IGNOTUM_PARTIAL;
    } else {
        ret = IGNOTUM_NONE;
    }

    return ret;
}
