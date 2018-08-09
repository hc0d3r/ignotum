#ifndef _IGN_STR_H_
#define _IGN_STR_H_

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

void parser(struct ignotum_mapinfo *out, const char *buf, int *i, int limit, int *flag, int *aux_len);

#endif
