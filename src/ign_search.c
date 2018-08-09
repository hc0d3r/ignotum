#include "ign_search.h"

size_t ignotum_search(ignotum_search_t *out, off_t remote_addr, const void *haystack, size_t hlen, const void *needle, size_t nlen){
    size_t i, j, k, ret = 0;

    if(!hlen || nlen > hlen){
        goto end;
    }

    for(i=0;i<hlen;i++){
        for(j=i, k=0; j<hlen && k<nlen && *(char *)(haystack+j) == *(char *)(needle+k); j++, k++){
            if(k == nlen-1){
                out->addrs = realloc(out->addrs, sizeof(off_t)*(out->len+1));
                out->addrs[out->len] = remote_addr+i;
                out->len++;
                ret++;
            }
        }
    }

    end:
        return ret;

}

void free_ignotum_search(ignotum_search_t *search_res){
    free(search_res->addrs);
    search_res->len = 0;
}
