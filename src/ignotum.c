/*
 * gcc ignotum.c -c -I. -Wall -Wextra -o ../lib/ignotum.o
 *
 */

#include <ignotum.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* size constant */
#define SIGNED_OVERFLOW_PID_T (pid_t)~((pid_t)1 << ((sizeof(pid_t)*8)-1)) /* 0b100000000000000... */
#define PATHNAME_LEN 1024
#define wordsize sizeof(long)

/* function macros */
#define check_hex_digit(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define ignotum_free(x) __safefree((void **)&(x))

typedef struct ignotum_string {
	char *ptr;
	size_t size;
} ignotum_string_t;

/* check if pointer is null, if not free and set it to null, to evite
 * wild pointers
 */
static void __safefree(void **pp){
	if(pp != NULL){
		free(*pp);
		*pp = NULL;
	}
}

static void ignotum_mem_search_alloc(ignotum_search_t *out){
	if(!out->len){
		out->addrs = malloc(sizeof(off_t));
		out->len = 1;
	} else {
		out->len++;
		out->addrs = realloc(out->addrs, sizeof(off_t)*out->len);
	}
}

ignotum_search_t *ignotum_search(off_t remote_addr, const void *haystack, size_t hlen, const void *needle, size_t nlen){
	ignotum_search_t *ret = NULL;
	size_t i, j, k;

	if(!hlen || nlen > hlen){
		goto end;
	}

	ret = calloc(1, sizeof(ignotum_search_t));

	for(i=0;i<hlen;i++){
		for(j=i, k=0; j<hlen && k<nlen && *(char *)(haystack+j) == *(char *)(needle+k); j++, k++){
			if(k == nlen-1){
				ignotum_mem_search_alloc(ret);
				ret->addrs[ret->len-1] = remote_addr+i;
			}

		}
	}

	if(!ret->len){
		free(ret);
		ret = NULL;
	}

	end:
		return ret;

}

static void ignotum_string_copy(ignotum_string_t *string, const char *src, size_t src_size){
	if(!string->size){
		string->ptr = malloc( sizeof(char) * (src_size+1) );
		memcpy(string->ptr, src, src_size);
		string->size = src_size;
		string->ptr[src_size] = 0;
	} else {
		string->ptr = realloc(string->ptr, string->size+1 + src_size);
		memcpy(string->ptr+string->size, src, src_size);
		string->size = src_size+string->size;
		string->ptr[string->size] = 0;
	}
}


ssize_t ignotum_mem_write(int mem_fd, const void *src, size_t n, off_t offset){
	return pwrite(mem_fd, src, n, offset);
}

ssize_t ignotum_mem_read(int mem_fd, void *out, size_t n, off_t offset){
	return pread(mem_fd, out, n, offset);
}

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

int ignotum_openmem(pid_t pid_number, int mode){
	char filename[32];
	if(pid_number == 0){
		memcpy(filename, "/proc/self/mem", 15);
	} else {
		sprintf(filename, "/proc/%d/mem", pid_number);
	}

	return open(filename, mode);
}

char hexchar(const char x){
	char ret = 0;

	if( x >= '0' && x <= '9' )
		ret = x-'0';

	else if( x >= 'a' && x <= 'f' )
		ret = x-'a'+10;

	else if( x >= 'A' && x <= 'F' )
		ret = x-'A'+10;

	return ret;
}

ssize_t ignotum_get_map_list(pid_t target_pid, ignotum_map_list_t **out){
	int parser_flags = ignotum_first_addr, v = 0, end = 0, maps_fd;
	ssize_t size, i = 0, j;
	char buf[1024];

	ssize_t ret = -1;

	if(target_pid)
		sprintf(buf, "/proc/%d/maps", target_pid);
	else
		memcpy(buf, "/proc/self/maps", 16);

	if((maps_fd = open(buf, O_RDONLY)) == -1){
		goto end;
	}

	ignotum_map_info_t tmp, *aux;
	ignotum_string_t aux_string;

	memset(&tmp, 0, sizeof(tmp));
	memset(&aux_string, 0, sizeof(aux_string));

	while( (size = read(maps_fd, buf, sizeof(buf))) > 0 ){
		for(i=0; i<size; i++){
			char c = buf[i];

			switch(parser_flags){
				case ignotum_first_addr:
					if(c != '-'){
						tmp.start_addr <<= 4;
						tmp.start_addr += hexchar(c);
					} else {
						parser_flags = ignotum_second_addr;
					}
				break;

				case ignotum_second_addr:
					if(c != ' '){
						tmp.end_addr <<= 4;
						tmp.end_addr += hexchar(c);
					} else {
						parser_flags = ignotum_flags;
					}
				break;

				case ignotum_flags:
					if(c == '-')
						v = 0;
					else if(c == 'r')
						v = ignotum_read;
					else if(c == 'x')
						v = ignotum_exec;
					else if(c == 'w')
						v = ignotum_write;
					else if(c == 'p')
						v = ignotum_private;
					else if(c == 's')
						v = ignotum_shared;
					else if(c == ' '){
						parser_flags = ignotum_offset;
						break;
					}

					tmp.perms |= v;
				break;

				case ignotum_offset:
					if(c != ' '){
						tmp.offset <<= 4;
						tmp.offset += hexchar(c);
					} else {
						parser_flags = ignotum_dev;
					}
				break;

				case ignotum_dev:
					if(c == ':'){
						break;
					}

					else if(c == ' '){
						parser_flags = ignotum_inode;
					}

					else {
						tmp.st_dev <<= 4;
						tmp.st_dev += hexchar(c);
					}
				break;

				case ignotum_inode:
					if(c != ' '){
						tmp.st_ino *= 10;
						tmp.st_ino += c-'0';
					} else {
						parser_flags = ignotum_skip_space;
					}
				break;

				case ignotum_skip_space:
					for(; i<size; i++){
						if(buf[i] == '\n'){
							aux = malloc(sizeof(ignotum_map_info_t));
							memcpy(aux, &tmp, sizeof(ignotum_map_info_t));

							*out = malloc(sizeof(ignotum_map_list_t));
							(*out)->map = aux;
							(*out)->next = NULL;
							out = &((*out)->next);

							parser_flags = ignotum_first_addr;
							memset(&tmp, 0, sizeof(ignotum_map_info_t));
							ret++;
							break;
						} else if(buf[i] != ' '){
							i--;
							parser_flags = ignotum_pathname;
							break;
						}
					}
				break;

				case ignotum_pathname:
					for(j=i; i<size; i++){
						if(buf[i] == '\n'){
							end = 1;
							break;
						}
					}

					ignotum_string_copy(&aux_string, &(buf[j]), i-j);

					if(end){
						aux = malloc(sizeof(ignotum_map_info_t));
						tmp.pathname = aux_string.ptr;

						memcpy(aux, &tmp, sizeof(ignotum_map_info_t));

						*out = malloc(sizeof(ignotum_map_list_t));
						(*out)->map = aux;
						(*out)->next = NULL;
						out = &((*out)->next);

						parser_flags = ignotum_first_addr;
						memset(&tmp, 0, sizeof(tmp));
						memset(&aux_string, 0, sizeof(aux_string));
						end = 0;
						ret++;
					}

				break;
			}
		}
	}

	close(maps_fd);

	end:
		return ret;
}

ignotum_map_info_t *ignotum_getmapbyaddr(pid_t pid, off_t addr){
	ignotum_map_info_t *ret = NULL;
	int parser_flags = ignotum_first_addr;
	int maps_fd, size, i, j, v = 0, end = 0;
	char buf[1024];

	if(pid)
		sprintf(buf, "/proc/%d/maps", pid);
	else
		memcpy(buf, "/proc/self/maps", 16);

	if((maps_fd = open(buf, O_RDONLY)) == -1){
		goto open_fail;
	}


	ignotum_map_info_t tmp;
	ignotum_string_t aux_string;

	memset(&tmp, 0, sizeof(tmp));
	memset(&aux_string, 0, sizeof(aux_string));

	while( (size = read(maps_fd, buf, sizeof(buf))) > 0 ){
		for(i=0; i<size; i++){
			char c = buf[i];

			switch(parser_flags){
				case ignotum_first_addr:
					if(c != '-'){
						tmp.start_addr <<= 4;
						tmp.start_addr += hexchar(c);
					} else {
						//printf("primeiro addr acabou ... %lx <= %lx ? %d\n", tmp.range.start_addr, addr, (tmp.range.start_addr <= addr));
						//getchar();
						if(tmp.start_addr <= addr){
							parser_flags = ignotum_second_addr;
						} else {
							tmp.start_addr = 0;
							parser_flags = ignotum_skip_line;
						}
					}
				break;

				case ignotum_second_addr:
					if(c != ' '){
						tmp.end_addr <<= 4;
						tmp.end_addr += hexchar(c);
					} else {
						if(addr <= tmp.end_addr){
							parser_flags = ignotum_flags;
						} else {
							tmp.start_addr = 0;
							tmp.end_addr = 0;
							parser_flags = ignotum_skip_line;
						}
					}
				break;

				case ignotum_flags:
					if(c == '-')
						v = 0;
					else if(c == 'r')
						v = ignotum_read;
					else if(c == 'x')
						v = ignotum_exec;
					else if(c == 'w')
						v = ignotum_write;
					else if(c == 'p')
						v = ignotum_private;
					else if(c == 's')
						v = ignotum_shared;
					else if(c == ' '){
						parser_flags = ignotum_offset;
						break;
					}

					tmp.perms |= v;
				break;

				case ignotum_offset:
					if(c != ' '){
						tmp.offset <<= 4;
						tmp.offset += hexchar(c);
					} else {
						parser_flags = ignotum_dev;
					}
				break;

				case ignotum_dev:
					if(c == ':'){
						break;
					}

					else if(c == ' '){
						parser_flags = ignotum_inode;
					}

					else {
						tmp.st_dev <<= 4;
						tmp.st_dev += hexchar(c);
					}
				break;

				case ignotum_inode:
					if(c != ' '){
						tmp.st_ino *= 10;
						tmp.st_ino += c-'0';
					} else {
						parser_flags = ignotum_skip_space;
					}
				break;

				case ignotum_skip_space:
					for(; i<size; i++){
						if(buf[i] == '\n'){
							ret = malloc(sizeof(ignotum_map_info_t));
							memcpy(ret, &tmp, sizeof(ignotum_map_info_t));
							goto end;

						} else if(buf[i] != ' '){
							i--;
							parser_flags = ignotum_pathname;
							break;
						}
					}
				break;

				case ignotum_pathname:
					for(j=i; i<size; i++){
						if(buf[i] == '\n'){
							end = 1;
							break;
						}
					}

					ignotum_string_copy(&aux_string, &(buf[j]), i-j);

					if(end){
						ret = malloc(sizeof(ignotum_map_info_t));
						memcpy(ret, &tmp, sizeof(ignotum_map_info_t));
						ret->pathname = aux_string.ptr;
						goto end;
					}

				break;

				case ignotum_skip_line:
					if(c == '\n'){
						parser_flags = ignotum_first_addr;
					}
				break;
			}
		}
	}



	end:
		close(maps_fd);
	open_fail:
		return ret;
}

void free_ignotum_map_list(ignotum_map_list_t **addr){
	ignotum_map_list_t *aux;

	while(*addr){
		aux = (*addr)->next;
		free_ignotum_map_info((*addr)->map);
		free(*addr);
		*addr = aux;
	}

	addr = NULL;

}

void free_ignotum_map_info(ignotum_map_info_t *info){
	free(info->pathname);
	free(info);
}

void free_ignotum_search(ignotum_search_t *search_res){
	if(search_res && search_res->len){
		ignotum_free(search_res->addrs);
		search_res->len = 0;
	}
}
