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
#define wordsize sizeof(long)

/* function macros */
#define check_hex_digit(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define ignotum_free(x) __safefree((void **)&(x))

enum {
	ignp_addr_start,
	ignp_addr_end,
	ignp_flags,
	ignp_offset,
	ignp_dev,
	ignp_ino,
	ignp_skip,
	ignp_pathname,
	ignp_end,
};

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

ssize_t ignotum_mem_write(pid_t pid, const void *src, size_t n, off_t offset){
	char pathbuf[32], *filename;
	ssize_t ret = -1;

	if(!pid){
		filename = "/proc/self/mem";
	} else {
		filename = pathbuf;
		sprintf(pathbuf, "/proc/%d/mem", pid);
	}

	int fd = open(filename, O_WRONLY);
	if(fd == -1){
		goto end;
	}

	ret = pwrite(fd, src, n, offset);
	close(fd);

	end:
		return ret;
}

ssize_t ignotum_mem_read(pid_t pid, void *out, size_t n, off_t offset){
	char pathbuf[32], *filename;
	ssize_t ret = -1;

	if(!pid){
		filename = "/proc/self/mem";
	} else {
		filename = pathbuf;
		sprintf(pathbuf, "/proc/%d/mem", pid);
	}

	int fd = open(filename, O_RDONLY);
	if(fd == -1){
		goto end;
	}

	ret = pread(fd, out, n, offset);
	close(fd);

	end:
		return ret;
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

int hexchar(const char c){
	if(c <= '9')
		return c-'0';
	else
		return c-'a'+10;
}

static void parser(struct ignotum_map_info *out, const char *buf, int *i, int limit,
	int *flag, int *aux_len){
	int aux = *i;
	size_t len, tmp;
	char c;

	while(aux<limit && *flag != ignp_end){
		switch(*flag){
			case ignp_addr_start:
				while(aux<limit){
					c = buf[aux++];

					if(c == '-'){
						*flag = ignp_addr_end;
						break;
					}

					out->start_addr <<= 4;
					out->start_addr |= hexchar(c);
				}
			break;

			case ignp_addr_end:
				while(aux<limit){
					c = buf[aux++];

					if(c == ' '){
						*flag = ignp_flags;
						break;
					}
					out->end_addr <<= 4;
					out->end_addr |= hexchar(c);
				}
			break;

			case ignp_flags:
				while(aux<limit){
					c = buf[aux++];
					if(c == ' '){
						*flag = ignp_offset;
						break;
					}

					out->perms <<= 1;
					if(c != '-' && c != 's'){
						out->perms |= 1;
					}
				}
			break;

			case ignp_offset:
				while(aux<limit){
					c = buf[aux++];

					if(c == ' '){
						*flag = ignp_dev;
						break;
					}

					out->offset <<= 4;
					out->offset |= hexchar(c);
				}
			break;

			case ignp_dev:
				while(aux<limit){
					c = buf[aux++];

					if(c == ' '){
						*flag = ignp_ino;
						break;
					}

					if(c == ':'){
						continue;
					}

					out->st_dev <<= 4;
					out->st_dev += hexchar(c);
				}
			break;

			case ignp_ino:
				while(aux<limit){
					c = buf[aux++];

					if(c == ' '){
						*flag = ignp_skip;
						break;
					}

					out->st_ino *= 10;
					out->st_ino += c-'0';
				}
			break;

			case ignp_skip:
				while(aux<limit){
					c = buf[aux++];
					if(c == ' '){
						continue;
					} else if(c == '\n'){
						*flag = ignp_end;
					} else {
						*flag = ignp_pathname;
						aux--;
					}
					break;
				}
			break;

			case ignp_pathname:
				tmp = aux;

				while(aux<limit){
					c = buf[aux++];
					if(c == '\n'){
						*flag = ignp_end;
						break;
					}
				}

				len = aux-tmp;
				if(*flag == ignp_end)
					len--;

				out->pathname = realloc(out->pathname, *aux_len+len+1);
				memcpy(&(out->pathname[*aux_len]), &buf[tmp], len);
				out->pathname[*aux_len+len] = 0x0;

				*aux_len += len;
			break;
		}
	}

	*i = aux;
}

ssize_t ignotum_get_map_list(pid_t target_pid, ignotum_map_list_t **out){
	int maps_fd, flag, i, size, aux_len;
	ignotum_map_info_t *info;
	char buf[1024];

	ssize_t ret = -1;

	if(target_pid)
		sprintf(buf, "/proc/%d/maps", target_pid);
	else
		memcpy(buf, "/proc/self/maps", 16);

	if((maps_fd = open(buf, O_RDONLY)) == -1){
		goto end;
	}

	info = calloc(1, sizeof(ignotum_map_info_t));
	flag = ignp_addr_start;
	aux_len = 0;
	ret = 0;

	while((size = read(maps_fd, buf, sizeof(buf))) > 0 ){
		for(i=0; i<size;){
			parser(info, buf, &i, size, &flag, &aux_len);
			if(flag == ignp_end){
				*out = malloc(sizeof(ignotum_map_list_t));
				(*out)->map = info;
				(*out)->next = NULL;
				out = &((*out)->next);

				info = calloc(1, sizeof(ignotum_map_info_t));
				flag = ignp_addr_start;
				aux_len = 0;
				ret++;
			}
		}
	}

	free(info);

	close(maps_fd);

	end:
		return ret;
}

ignotum_map_info_t *ignotum_getmapbyaddr(pid_t pid, off_t addr){
	int maps_fd, flag, i, size, aux_len;
	ignotum_map_info_t *tmp, *ret = NULL;
	char buf[1024];

	if(pid)
		sprintf(buf, "/proc/%d/maps", pid);
	else
		memcpy(buf, "/proc/self/maps", 16);

	if((maps_fd = open(buf, O_RDONLY)) == -1){
		goto open_fail;
	}


	tmp = calloc(1, sizeof(ignotum_map_info_t));
	flag = ignp_addr_start;
	aux_len = 0;

	while((size = read(maps_fd, buf, sizeof(buf))) > 0){
		for(i=0; i<size;){
			parser(tmp, buf, &i, size, &flag, &aux_len);
			if(flag == ignp_end){
				if(tmp->start_addr <= addr && addr <= tmp->end_addr){
					ret = tmp;
					goto end;
				}

				memset(tmp, 0x0, sizeof(ignotum_map_info_t));
				flag = ignp_addr_start;
				aux_len = 0;

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
