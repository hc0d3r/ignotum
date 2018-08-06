#include <ignotum.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(void){
	size_t j, ret;

	char test[]="abcdefghijklmnopq ------ leet --------";
	(void)test;

	ignotum_maplist_t *addrs = NULL, *i = NULL;
	ignotum_search_t result = DEFAULT_IGNOTUM_MEMSEARCH;

	printf("char test[] = %p\n", test);

	if( ignotum_getmaplist(0, &addrs) ){
		for(i=addrs; i!=NULL; i=i->next){
			if(i->map->pathname == NULL)
				continue;

			if(!strcmp("[stack]", i->map->pathname)){
				size_t len = i->map->end_addr-i->map->start_addr;
				char *data = malloc(len);

				len = ignotum_mem_read(0, data, len, i->map->start_addr);
				ret = ignotum_search(&result, i->map->start_addr, data, len, "leet", 4);

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
