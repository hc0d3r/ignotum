#include <ignotum.h>

int main(void){
	int mem_fd;
	size_t j;

	char test[]="abcdefghijklmnopq ------ leet --------";
	(void)test;

	ignotum_map_list_t *addrs = NULL, *i = NULL;
	ignotum_search_t *result = NULL;
	// = DEFAULT_IGNOTUM_MEMSEARCH;

	printf("char test[] = %p\n", test);

	if( ignotum_get_map_list(0, &addrs) ){
		for(i=addrs; i!=NULL; i=i->next){
			if(i->map->pathname == NULL)
				continue;

			if(!strcmp("[stack]", i->map->pathname)){
				mem_fd = ignotum_openmem(0, O_RDONLY);
				size_t len = i->map->range.end_addr-i->map->range.start_addr;
				char *data = malloc(len);

				len = ignotum_mem_read(mem_fd, data, len, i->map->range.start_addr);
				result = ignotum_search(i->map->range.start_addr, data, len, "leet", 4);

				if(result){
					for(j=0; j<result->len; j++){
						printf("leet found at =: %zx | %s\n", result->addrs[j], (char *)result->addrs[j] );
					}

					free_ignotum_search(result);
				}

				close(mem_fd);
				break;
			}
		}
	}

	free_ignotum_map_list(&addrs);
	return 0;

}
