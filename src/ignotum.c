/*
 * gcc ignotum.c -c -I. -Wall -Wextra -o ../lib/ignotum.o
 *
 */

#include <ignotum.h>

/* check if pointer is null, if not free and set it to null, to evite
 * wild pointers
 */
static void __safefree(void **pp){
	if(pp != NULL){
		free(*pp);
		*pp = NULL;
	}
}

static void ignotum_mem_search_alloc(ignotum_mem_search_t *out){
	if(!out->len){
		out->addrs = malloc(sizeof(off_t));
		out->len = 1;
	} else {
		out->len++;
		out->addrs = realloc(out->addrs, sizeof(off_t)*out->len);
	}
}

ssize_t ignotum_mem_search(int mem_fd, const void *search, size_t search_size, ignotum_addr_range_t range, ignotum_mem_search_t *out){
	ssize_t ret = -1;
	size_t i, j, k, string_len;

	size_t len = (size_t)(range.end_addr-range.start_addr);
	if(!len || search_size > len){
		return -1;
	}

	char *ptr = malloc(len);
	if(ptr == NULL){
		goto end;
	}

	ret = pread(mem_fd, ptr, len, range.start_addr);

	if(ret <= 0){
		goto end;
	}

	string_len = (size_t)ret;

	for(i=0;i<string_len;i++){
		for(j=i, k=0; j<string_len && k<search_size && *(ptr+j) == *(char *)(search+k); j++, k++ ){
			if(k == search_size-1){
				ignotum_mem_search_alloc(out);
				out->addrs[out->len-1] = range.start_addr+i;
			}
		}
	}

	ignotum_free(ptr);

	end:
		return ret;

}

static void ignotum_string_copy(ignotum_string_t *string, const char *src, size_t src_size){
	if(!string->size){
		string->ptr = malloc( sizeof(char) * (src_size+1) );
		strncpy(string->ptr, src, src_size);
		string->size = src_size;
		string->ptr[src_size] = 0;
	} else {
		string->ptr = realloc(string->ptr, string->size+1 + src_size);
		strncpy(string->ptr+string->size, src, src_size);
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

size_t ignotum_ptrace_write(pid_t pid, const void *data, long addr, size_t len){
	size_t i, ret = 0;
	long old_data, new_data = 0L;

	for(i=0; (i+wordsize)<len; i+=wordsize){
		ptrace(PTRACE_POKEDATA, pid, addr+i, *(long *)(data+i));
		if(errno)
			return ret;

		ret += wordsize;
	}

	len -= i;

	if(len == wordsize){
		new_data = *(long *)(data+i);
	} else {
		memcpy(&new_data, data+i, len);
		old_data = ptrace(PTRACE_PEEKDATA, pid, addr+i, 0L);
		old_data &= (unsigned long)-1 << (8*len);
		new_data |= old_data;
	}

	ptrace(PTRACE_POKEDATA, pid, addr+i, new_data);
	if(!errno)
		ret += len;


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

	ret = (wordsize > n) ? n : (wordsize-offset);

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
	char filename[6 + MAX10_PID_T_STR + 5];
	if(pid_number == 0){
		memcpy(filename, "/proc/self/mem", 15);
	} else {
		snprintf(filename, sizeof(filename), "/proc/%d/mem", pid_number);
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
	char buff[1024];

	ssize_t ret = -1;

	if(target_pid)
		snprintf(buff, sizeof(buff), "/proc/%d/maps", target_pid);
	else
		memcpy(buff, "/proc/self/maps", 16);

	if((maps_fd = open(buff, O_RDONLY)) == -1){
		goto end;
	}

	ignotum_map_info_t tmp, *aux;
	memset(&tmp, 0, sizeof(tmp));

	while( (size = read(maps_fd, buff, sizeof(buff))) > 0 ){
		for(i=0; i<size; i++){
			char c = buff[i];

			switch(parser_flags){
				case ignotum_first_addr:
					if(c != '-'){
						tmp.range.start_addr <<= 4;
						tmp.range.start_addr += hexchar(c);
					} else {
						parser_flags = ignotum_second_addr;
					}
				break;

				case ignotum_second_addr:
					if(c != ' '){
						tmp.range.end_addr <<= 4;
						tmp.range.end_addr += hexchar(c);
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
						if(buff[i] == '\n'){
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
						} else if(buff[i] != ' '){
							i--;
							parser_flags = ignotum_pathname;
							break;
						}
					}
				break;

				case ignotum_pathname:
					for(j=i; i<size; i++){
						if(buff[i] == '\n'){
							end = 1;
							break;
						}
					}

					ignotum_string_copy(&tmp.pathname, &(buff[j]), i-j);

					if(end){
						aux = malloc(sizeof(ignotum_map_info_t));
						memcpy(aux, &tmp, sizeof(ignotum_map_info_t));

						*out = malloc(sizeof(ignotum_map_list_t));
						(*out)->map = aux;
						(*out)->next = NULL;
						out = &((*out)->next);

						parser_flags = ignotum_first_addr;
						memset(&tmp, 0, sizeof(tmp));
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
		snprintf(buf, sizeof(buf), "/proc/%d/maps", pid);
	else
		memcpy(buf, "/proc/self/maps", 16);

	if((maps_fd = open(buf, O_RDONLY)) == -1){
		goto end;
	}


	ignotum_map_info_t tmp;
	memset(&tmp, 0, sizeof(tmp));

	while( (size = read(maps_fd, buf, sizeof(buf))) > 0 ){
		for(i=0; i<size; i++){
			char c = buf[i];

			switch(parser_flags){
				case ignotum_first_addr:
					if(c != '-'){
						tmp.range.start_addr <<= 4;
						tmp.range.start_addr += hexchar(c);
					} else {
						//printf("primeiro addr acabou ... %lx <= %lx ? %d\n", tmp.range.start_addr, addr, (tmp.range.start_addr <= addr));
						//getchar();
						if(tmp.range.start_addr <= addr){
							parser_flags = ignotum_second_addr;
						} else {
							tmp.range.start_addr = 0;
							parser_flags = ignotum_skip_line;
						}
					}
				break;

				case ignotum_second_addr:
					if(c != ' '){
						tmp.range.end_addr <<= 4;
						tmp.range.end_addr += hexchar(c);
					} else {
						if(addr <= tmp.range.end_addr){
							parser_flags = ignotum_flags;
						} else {
							tmp.range.start_addr = 0;
							tmp.range.end_addr = 0;
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

					ignotum_string_copy(&tmp.pathname, &(buf[j]), i-j);

					if(end){
						ret = malloc(sizeof(ignotum_map_info_t));
						memcpy(ret, &tmp, sizeof(ignotum_map_info_t));
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
	free(info->pathname.ptr);
	free(info);
}

void free_ignotum_mem_search(ignotum_mem_search_t *search_res){
	if(search_res->len){
		ignotum_free(search_res->addrs);
		search_res->len = 0;
	}
}
