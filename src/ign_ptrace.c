#include "ign_ptrace.h"

size_t ignotum_ptrace_write(pid_t pid, const void *data, long addr, size_t n){
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
		ptrace(PTRACE_POKEDATA, pid, aligned_addr, *(long *)data);
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

		memcpy(((char *)(&new_bytes)+offset), data, aux);
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
			memcpy(&new_bytes, data+ret, aux);

			old_bytes &= -1UL << (aux * 8);
			new_bytes |= old_bytes;

			ptrace(PTRACE_POKEDATA, pid, aligned_addr, new_bytes);
			if(errno)
				goto end;

			ret += aux;

			break;
		}

		ptrace(PTRACE_POKEDATA, pid, aligned_addr, *(long *)(data+ret));
		if(errno)
			goto end;

		ret += wordsize;
	}

	end:
		return ret;
}


size_t ignotum_ptrace_read(pid_t pid, void *output, long addr, size_t n){
	long aligned_addr, bytes;
	size_t offset, ret = 0;

	aligned_addr = addr & (long)(-wordsize);
	offset = addr - aligned_addr;

	bytes = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0L);
	if(errno)
		goto end;

	ret = wordsize-offset;
	if(ret > n)
		ret = n;

	memcpy(output, ((char *)(&bytes)+offset), ret);
	aligned_addr += wordsize;


	while(ret < n){
		bytes = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0L);
		if(errno)
			goto end;

		if((ret+wordsize) > n){
			size_t tmp = n-ret;
			memcpy(output+ret, &bytes, tmp);
			ret += tmp;
			break;
		}

		*(long *)(output+ret) = bytes;

		ret += wordsize;
		aligned_addr += wordsize;
	}

	end:
		return ret;
}
