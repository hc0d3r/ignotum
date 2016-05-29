/*
 * gcc ignotum.c -c -I. -Wall -Wextra -o ../lib/ignotum.o
 *
 */

#include <ignotum.h>

static const char *memory_files[] = {"maps", "mem"};

/* check if pointer is null, if not free and set it to null, to evite
 * wild pointers
 */
static void __safefree(void **pp){
	if(pp != NULL){
		free(*pp);
		*pp = NULL;
	}
}

const char *ignotum_strerror(ignotum_status code){
	static const char *str_err[]={
		"success",
		"invalid pid number",
		"failed to open map",
		"failed to open mem",
		"pid number overflow",
	};

	return (code < IGNOTUM_ERR_SIZE) ? str_err[code] : "invalid error code";
}

void strrev(char *dest, const char *src, size_t size){
	size_t i = 0;

	if(!size)   /* check if size is zero, to avoid size turn (size_t)-1 and stack overflow */
		return;

	size--;

	while(i <= size){
		dest[i] = src[size-i];
		i++;
	}

	dest[i] = 0;
}

static char *ignotum_pid2str(pid_t pid_number){
	static char str_pid[MAX10_PID_T_STR];
	char s_aux[MAX10_PID_T_STR];
	size_t i = 0;

	if(pid_number == 0){
		str_pid[0] = '0';
		str_pid[1] = 0x0;
		return str_pid;
	}

	while(pid_number && i != MAX10_PID_T_STR){ /* size verification, to avoid potential overflow bugs */
		s_aux[i++] = pid_number%10 + '0';
		pid_number /= 10;
	}

	s_aux[i] = 0;

	strrev(str_pid, s_aux, i);

	return str_pid;
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

int ignotum_memsearch(const void *search, size_t search_size, int mem_fd, ignotum_addr_range_t range, ignotum_mem_search_t *out){
	int ret;
	size_t i, j, k, string_len;

	size_t len = (size_t)(range.end_addr-range.start_addr);
	if(!len || search_size > len){
		return -1;
	}

	char *ptr = malloc(len);
	if(ptr == NULL){
		return -1;
	}

	lseek(mem_fd, range.start_addr, SEEK_SET);
	ret = read(mem_fd, ptr, len);

	if(ret <= 0 || (size_t)ret < len){
		return -1;
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

	return ret;

}

static char *genpath(char *proc_path, const char *pid_str, const char *filename){
	size_t i;

	for(i=6; *pid_str; i++, pid_str++)
		proc_path[i] = *pid_str;

	proc_path[i++] = '/';

	for(; *filename; filename++, i++)
		proc_path[i] = *filename;

	proc_path[i] = 0x0;

	return proc_path;
}

ignotum_status ignotum_openmap(pid_t pid_number, int *fd_out){
	int tmp_fd;
	char *pid_str;
	char proc_path[6 + MAX10_PID_T_STR + 5] = "/proc/";

	if(pid_number < 0){
		return IGNOTUM_INVALID_PID_NUMBER;
	}

	pid_str = ignotum_pid2str(pid_number);
	tmp_fd = open(genpath(proc_path, pid_str, memory_files[0]), O_RDONLY);

	if( tmp_fd == -1){
		return IGNOTUM_OPEN_MAP_FAILED;
	} else {
		*fd_out = tmp_fd;
	}

	return IGNOTUM_SUCCESS;
}

static void ignotum_string_t_copy(ignotum_string_t *string, const char *src, size_t src_size){
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


static ignotum_status str2pid_t(const char *pid_str, pid_t *out){
	pid_t tmp = 0;
	char aux;

	if(!strcmp("self", pid_str)){
		*out = getpid();
		return IGNOTUM_SUCCESS;
	}

	while(*pid_str){
		aux = *pid_str;

		if(aux < '0' || aux > '9')
			return IGNOTUM_INVALID_PID_NUMBER;
		aux -= '0';

		if(tmp > SIGNED_OVERFLOW_PID_T/10){
			return IGNOTUM_PID_T_OVERFLOW;
		} else {
			tmp *= 10;
		}

		if(tmp > SIGNED_OVERFLOW_PID_T - aux){
			return IGNOTUM_PID_T_OVERFLOW;
		} else {
			tmp += aux;
		}

		pid_str++;
	}

	*out = tmp;

	return IGNOTUM_SUCCESS;

}

int checkpidstring(const char *str){

	if(!strcmp("self", str)){
		return 1;
	}

	for(; *str; str++){
		if(*str < '0' || *str > '9')
			return 0;
	}

	return 1;
}

ignotum_status ignotum_openmapstr(const char *pid_str, int *fd_out){
	char proc_path[6 + MAX10_PID_T_STR + 5] = "/proc/";
	int tmp_fd;

	if( checkpidstring(pid_str) ){
		tmp_fd = open(genpath(proc_path, pid_str, memory_files[0]), O_RDONLY);

		if(tmp_fd == -1){
			return IGNOTUM_OPEN_MAP_FAILED;
		} else {
			*fd_out = tmp_fd;
		}
	} else {
		return IGNOTUM_INVALID_PID_STR;
	}

	return IGNOTUM_SUCCESS;
}

int ignotum_memwrite(int mem_fd, off_t offset, const void *src, size_t n){
	lseek(mem_fd, offset, SEEK_SET);
	return write(mem_fd, src, n);
}

int ignotum_memread(int mem_fd, off_t offset, void *out, size_t n){
	lseek(mem_fd, offset, SEEK_SET);
	return read(mem_fd, out, n);
}

ignotum_status ignotum_openmem(pid_t pid_number, int *fd_out, int mode, int attach_pid){
	int tmp_fd;
	char *pid_str;
	char proc_path[6 + MAX10_PID_T_STR + 5] = "/proc/";

	if(pid_number < 0){
		return IGNOTUM_INVALID_PID_NUMBER;
	}

	if(attach_pid){
		ptrace(PTRACE_ATTACH, pid_number, NULL, NULL);
		waitpid(pid_number, NULL, 0);
	}

	pid_str = ignotum_pid2str(pid_number);
	tmp_fd = open(genpath(proc_path, pid_str, memory_files[1]), mode);

	if( tmp_fd == -1){
		return IGNOTUM_OPEN_MEM_FAILED;
	} else {
		*fd_out = tmp_fd;
	}

	return IGNOTUM_SUCCESS;
}

ignotum_status ignotum_openmemstr(const char *pid_str, int *fd_out, int mode, int attach_pid){
	int tmp_fd;
	pid_t pid_number = 0;
	char proc_path[6 + MAX10_PID_T_STR + 5] = "/proc/";
	ignotum_status status;

	status = str2pid_t(pid_str, &pid_number);
	if( status == IGNOTUM_SUCCESS ){

		if(attach_pid){
			ptrace(PTRACE_ATTACH, pid_number, NULL, NULL);
			waitpid(pid_number, NULL, 0);
		}

		tmp_fd = open(genpath(proc_path, pid_str, memory_files[1]), mode);

		if(tmp_fd == -1){
			return IGNOTUM_OPEN_MEM_FAILED;
		} else {
			*fd_out = tmp_fd;
		}
	} else {
		return status;
	}

	return IGNOTUM_SUCCESS;
}

char hexchar(char x){
	if(x >= '0' && x <= '9')
	   	x = x-'0';

	else if(x >= 'a' && x <='f')
		x = x-'a'+10;

	else if(x >= 'A' && x <='F')
		x = x-'A'+10;

	return x;
}


size_t ignotum_getmappedaddr(int maps_fd, ignotum_mapped_addr_t **out){
	int size, i, allocated = 0, start_path = 0;
	int j = 0;

	size_t ret = 0;

	ignotum_mapped_addr_t **tmp = out, *aux = NULL;
	ignotum_elements_t elements = DEFAULT_IGNOTUM_ELEMENTS;

	char pathname[PATHNAME_LEN], buf[1024];

	while( (size = read(maps_fd, buf, 1024)) > 0 ){
		for(i=0; i<size; i++){
			if(elements.first_hex && !allocated){
				aux = calloc(1,sizeof(ignotum_mapped_addr_t));
				allocated = 1;
			}


			if(elements.first_hex){
				if(buf[i] == '-'){
					elements.second_hex = 1;
					elements.first_hex = 0;
				}

				else if(check_hex_digit(buf[i])){
					aux->range.start_addr = (aux->range.start_addr << 4) | hexchar(buf[i]);
				} else {
					goto ignotum_getmappedaddr_err;
				}

				continue;

			}

			if(elements.second_hex){
				if(check_hex_digit(buf[i])){
					aux->range.end_addr = (aux->range.end_addr << 4) | hexchar(buf[i]);
				}

				else if(buf[i] == ' '){
					elements.second_hex = 0;
					elements.perms = 1;
				}

				 else {
					goto ignotum_getmappedaddr_err;
				}

				continue;
			}

			if(elements.perms){
				if(buf[i] == ' '){
					elements.perms = 0;
					elements.offset = 1;
				} else if(buf[i] == 'r'){
					aux->perms |= 1;
				} else if(buf[i] == 'w'){
					aux->perms |= 2;
				} else if(buf[i] == 'x'){
					aux->perms |= 4;
				} else if(buf[i] == 'p'){
					aux->perms |= 8;
				} else if(buf[i] == 's'){
					aux->perms |= 16;
				} else if(buf[i] == '-'){
					continue;
				} else {
					goto ignotum_getmappedaddr_err;
				}

				continue;
			}

			if(elements.offset){
				if(buf[i] == ' '){
					elements.offset = 0;
					elements.dev = 1;
				} else if(check_hex_digit(buf[i])){
					aux->offset = (aux->offset << 4) | hexchar(buf[i]);
				} else {
					goto ignotum_getmappedaddr_err;
				}

				continue;
			}

			if(elements.dev){
				if(check_hex_digit(buf[i])){
					aux->st_dev = (aux->st_dev << 4) | hexchar(buf[i]);
				} else if(buf[i] == ':'){
					continue;
				} else if(buf[i] == ' '){
					elements.dev = 0;
					elements.inode = 1;
				}

				continue;
			}

			if(elements.inode){
				if(buf[i] == ' '){
					elements.inode = 0;
					elements.pathname = 1;
				} else if(buf[i] < '0' || buf[i] > '9'){
					goto ignotum_getmappedaddr_err;
				} else {
					aux->st_ino = 10*aux->st_ino + (buf[i]-'0');
				}

				continue;
			}

			if(elements.pathname){
				for(; i<size; i++){
					if(buf[i] == ' ' && !start_path){
						continue;
					}

					if(buf[i] == '\n'){
						elements.pathname = 0;
						elements.first_hex = 1;
						allocated = 0;
						if(j){
							ignotum_string_t_copy(&aux->pathname, pathname, j);
							j = 0;
						}

						*tmp = aux;
						tmp = &(aux->next);

						start_path = 0;
						i++;
						ret++;

						break;
					}

					else {
						start_path = 1;
						pathname[j] = buf[i];
						j++;
						if(j == PATHNAME_LEN){
							ignotum_string_t_copy(&aux->pathname, pathname, PATHNAME_LEN);
							j = 0;
						}
					}

				}
			}

		}

	}

	return ret;

	ignotum_getmappedaddr_err:
		ignotum_free(aux);
		return ret;

}

void free_ignotum_mapped_addr_t(ignotum_mapped_addr_t **addr){
	ignotum_mapped_addr_t *aux;

	while(*addr){
		aux = (*addr)->next;
		ignotum_free((*addr)->pathname.ptr);
		ignotum_free(*addr);
		*addr = aux;
	}

	addr = NULL;

}

void free_ignotum_mem_search(ignotum_mem_search_t *search_res){
	if(search_res->len){
		ignotum_free(search_res->addrs);
		search_res->len = 0;
	}
}
