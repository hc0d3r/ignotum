#include <ignotum.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main(void){
    static const char change_me[]="You cannot change me !!!";

    printf("(%p) before -> %s\n", change_me, change_me);

    ignotum_mem_write(0, "Yes, i can", 11, (off_t)change_me);

    printf("(%p) after  -> %s\n", change_me, change_me);

    return 0;

}
