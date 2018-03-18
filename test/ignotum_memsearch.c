#include <ignotum.h>

int main(void){
	int mem_fd;
	size_t j;

	char test[]="abcdefghijklmnopq ------ leet --------";
	(void)test;

	ignotum_mapped_addr_t *addrs = NULL, *i = NULL;
	ignotum_mem_search_t result = DEFAULT_IGNOTUM_MEMSEARCH;

	printf("char test[] = %p\n", test);

	if( ignotum_getmappedaddr(0, &addrs) ){
		for(i=addrs; i!=NULL; i=i->next){
			if(i->pathname.ptr == NULL)
				continue;

			if(!strcmp("[stack]", i->pathname.ptr)){
				mem_fd = ignotum_openmem(getpid(), O_RDONLY);
				ignotum_memsearch("leet", 4, mem_fd , i->range, &result);

				if(result.len){
					for(j=0; j<result.len; j++){
						printf("leet found at =: %zx | %s\n", result.addrs[j], (char *)result.addrs[j] );
					}
				}

				close(mem_fd);
				break;
			}
		}
	}

	free_ignotum_mapped_addr_t(&addrs);
	free_ignotum_mem_search(&result);

	return 0;

}
