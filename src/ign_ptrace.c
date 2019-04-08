#include <sys/ptrace.h>
#include <string.h>
#include <errno.h>

#include "ignotum.h"

#define wordsize sizeof(long)

size_t ignotum_ptrace_write(pid_t pid, const void *buf, size_t n, long addr){
    long aligned_addr, old_bytes, new_bytes;
    size_t offset, ret = 0, aux;
    unsigned long bitmask;

    if(!n)
        goto end;

    aligned_addr = addr & (long)(-wordsize);
    offset = addr - aligned_addr;

    aux = wordsize-offset;
    if(aux > n)
        aux = n;

    if(aux == wordsize){
        ptrace(PTRACE_POKEDATA, pid, aligned_addr, *(long *)buf);
        if(errno)
            goto end;
    } else {
        old_bytes = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0L);
        if(errno)
            goto end;

        new_bytes = 0;

        if(!offset)
            bitmask = 0;
        else
            bitmask = -1UL >> (wordsize-offset)*8;

        if((aux+offset) < wordsize){
            bitmask |= (~bitmask) << (aux * 8);
        }

        old_bytes &= bitmask;

        memcpy(((char *)(&new_bytes)+offset), buf, aux);
        new_bytes |= old_bytes;

        ptrace(PTRACE_POKEDATA, pid, aligned_addr, new_bytes);
        if(errno)
            goto end;
    }

    ret = aux;

    while(ret < n){
        aligned_addr += wordsize;

        if((ret+wordsize) > n){
            aux = n-ret;

            old_bytes = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0L);
            if(errno)
                goto end;

            new_bytes = 0;
            memcpy(&new_bytes, buf+ret, aux);

            old_bytes &= -1UL << (aux * 8);
            new_bytes |= old_bytes;

            ptrace(PTRACE_POKEDATA, pid, aligned_addr, new_bytes);
            if(errno)
                goto end;

            ret += aux;

            break;
        }

        ptrace(PTRACE_POKEDATA, pid, aligned_addr, *(long *)(buf+ret));
        if(errno)
            goto end;

        ret += wordsize;
    }

    end:
        return ret;
}


ssize_t ignotum_ptrace_read(pid_t pid, void *buf, size_t n, long addr){
    ssize_t ret;
    size_t nread = 0, pos = 0, len;

    long aligned, offset, bytes;

    if(n == 0){
        ret = 0;
        goto end;
    }

    if(addr & (wordsize-1)){
        aligned = addr & (long)(-wordsize);
        offset = addr - aligned;
        len = wordsize-offset;
        addr = aligned;
    } else {
        len = wordsize;
        offset = 0;
    }

    while(nread<n){
        bytes = ptrace(PTRACE_PEEKDATA, pid, addr, 0L);
        if(errno)
            break;

        nread += len;
        if(nread > n){
            len = n-(nread-len);
            nread = n;
        }

        if(len == wordsize){
            *(long *)(buf+pos) = bytes;
        } else {
            memcpy((char *)buf+pos, (char *)&bytes+offset, len);
            len = wordsize;
            offset = 0;
        }

        pos = nread;
        addr += wordsize;
    }

    if(!nread){
        ret = -1;
    } else {
        ret = (ssize_t)nread;
    }


    end:
    return ret;
}
