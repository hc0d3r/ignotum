#include "ign_search.h"

static void ignotum_mem_search_alloc(ignotum_search_t *out){
	if(!out->len){
		out->addrs = malloc(sizeof(off_t));
		out->len = 1;
	} else {
		out->len++;
		out->addrs = realloc(out->addrs, sizeof(off_t)*out->len);
	}
}

ignotum_search_t *ignotum_search(off_t remote_addr, const void *haystack, size_t hlen, const void *needle, size_t nlen){
	ignotum_search_t *ret = NULL;
	size_t i, j, k;

	if(!hlen || nlen > hlen){
		goto end;
	}

	ret = calloc(1, sizeof(ignotum_search_t));

	for(i=0;i<hlen;i++){
		for(j=i, k=0; j<hlen && k<nlen && *(char *)(haystack+j) == *(char *)(needle+k); j++, k++){
			if(k == nlen-1){
				ignotum_mem_search_alloc(ret);
				ret->addrs[ret->len-1] = remote_addr+i;
			}

		}
	}

	if(!ret->len){
		free(ret);
		ret = NULL;
	}

	end:
		return ret;

}

void free_ignotum_search(ignotum_search_t *search_res){
	if(search_res && search_res->len){
		free(search_res->addrs);
		search_res->len = 0;
	}
}
