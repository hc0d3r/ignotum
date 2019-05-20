#include "ignotum.h"
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    const char *buf;
    int i;
    int limit;
    int flag;
    int aux_len;
} parser_t;

static int hexchar(char c){
    if(c >= '0' && c <= '9')
        c = c-'0';
    else
        c = c-'a'+10;

    return c;
}

static void parser(struct ignotum_mapinfo *out, parser_t *info){
    int aux = info->i;
    size_t len, tmp;
    char c;

    while(aux<info->limit && info->flag != ignp_end){
        switch(info->flag){
            case ignp_addr_start:
                while(aux < info->limit){
                    c = info->buf[aux++];

                    if(c == '-'){
                        info->flag = ignp_addr_end;
                        break;
                    }

                    out->start_addr <<= 4;
                    out->start_addr |= hexchar(c);
                }
            break;

            case ignp_addr_end:
                while(aux < info->limit){
                    c = info->buf[aux++];

                    if(c == ' '){
                        info->flag = ignp_flags;
                        break;
                    }
                    out->end_addr <<= 4;
                    out->end_addr |= hexchar(c);
                }
            break;

            case ignp_flags:
                while(aux < info->limit){
                    c = info->buf[aux++];
                    if(c == ' '){
                        info->flag = ignp_offset;
                        break;
                    }


                    if(c == 'p'){
                        out->is_p = 1;
                        continue;
                    } else if(c == 's'){
                        out->is_s = 1;
                        continue;
                    }

                    out->perms <<= 1;

                    // r - 4 | w - 2 | x - 1
                    if(c != '-'){
                        out->perms |= 1;
                    }
                }
            break;

            case ignp_offset:
                while(aux < info->limit){
                    c = info->buf[aux++];

                    if(c == ' '){
                        info->flag = ignp_dev;
                        break;
                    }

                    out->offset <<= 4;
                    out->offset |= hexchar(c);
                }
            break;

            case ignp_dev:
                while(aux < info->limit){
                    c = info->buf[aux++];

                    if(c == ' '){
                        info->flag = ignp_ino;
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
                while(aux<info->limit){
                    c = info->buf[aux++];

                    if(c == ' '){
                        info->flag = ignp_skip;
                        break;
                    }

                    out->st_ino *= 10;
                    out->st_ino += c-'0';
                }
            break;

            case ignp_skip:
                while(aux < info->limit){
                    c = info->buf[aux++];
                    if(c == ' '){
                        continue;
                    } else if(c == '\n'){
                        info->flag = ignp_end;
                    } else {
                        info->flag = ignp_pathname;
                        aux--;
                    }
                    break;
                }
            break;

            case ignp_pathname:
                tmp = aux;

                while(aux < info->limit){
                    c = info->buf[aux++];
                    if(c == '\n'){
                        info->flag = ignp_end;
                        break;
                    }
                }

                len = aux-tmp;
                if(info->flag == ignp_end)
                    len--;

                out->pathname = realloc(out->pathname, info->aux_len+len+1);
                memcpy(&(out->pathname[info->aux_len]), info->buf+tmp, len);
                out->pathname[info->aux_len+len] = 0x0;

                info->aux_len += len;
            break;
        }
    }

    info->i = aux;
}
