#include "ign_str.h"

static int hexchar(const char c){
	if(c <= '9')
		return c-'0';
	else
		return c-'a'+10;
}

void parser(struct ignotum_map_info *out, const char *buf, int *i, int limit,
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
