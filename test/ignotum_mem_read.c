/* gcc -c test-ignotum_memread.c -o bin/test-ignotum_memread.o -Wall -Wextra
 * nasm -f elf64 read_my_nops.asm -o bin/read_my_nops.o
 * gcc bin/test-ignotum_memread.o bin/read_my_nops.o ../lib/ignotum.o -o bin/test-ignotum_memread
 */

#include <ignotum.h>

void read_my_nops(void);

int main(void){
	int mem_fd, i;
	unsigned char nops[10];

	mem_fd = ignotum_openmem(getpid(), O_RDWR);
	ignotum_mem_read(mem_fd, (off_t)read_my_nops, nops, 10);

	for(i=0; i<10; i++){
		printf("%x ", nops[i]);
	}

	putchar('\n');

	close(mem_fd);

	return 0;
}
