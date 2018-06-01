#include <ignotum.h>
#include <sys/mman.h>


int main(void){
	ignotum_map_list_t *addrs = NULL, *i = NULL;

	mmap((void *)0x13370000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0L);

	/* 0 will open /proc/self/maps */
	if( ignotum_get_map_list(0, &addrs) > 0 ){
		for(i=addrs; i!=NULL; i=i->next){
			printf("start-address-> %zx | end-address-> %zx\n", i->map->start_addr, i->map->end_addr);
		}
	}

	free_ignotum_map_list(&addrs);

	return 0;

}
