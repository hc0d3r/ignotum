#include <ignotum.h>
#include <sys/mman.h>


int main(void){
    ignotum_mapped_addr_t *addrs = NULL, *i = NULL;

	mmap((void *)0x13370000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0L);

	/* 0 will open /proc/self/maps */
    if( ignotum_getmappedaddr(0, &addrs) > 0 ){
        for(i=addrs; i!=NULL; i=i->next){
            printf("start-address-> %zx | end-address-> %zx\n", i->range.start_addr, i->range.end_addr);
        }
    }

    free_ignotum_mapped_addr_t(&addrs);

    return 0;

}
