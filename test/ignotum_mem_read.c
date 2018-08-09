/* gcc -c test-ignotum_memread.c -o bin/test-ignotum_memread.o -Wall -Wextra
 * nasm -f elf64 read_my_nops.asm -o bin/read_my_nops.o
 * gcc bin/test-ignotum_memread.o bin/read_my_nops.o ../lib/ignotum.o -o bin/test-ignotum_memread
 */

#include <ignotum.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

void read_my_nops(void);

int main(void){
    int i;
    unsigned char nops[10];

    ignotum_mem_read(0, nops, 10, (off_t)read_my_nops);

    for(i=0; i<10; i++){
        printf("%x ", nops[i]);
    }

    putchar('\n');

    return 0;
}
