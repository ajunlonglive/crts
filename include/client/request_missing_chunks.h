#ifndef __REQUEST_MISSING_CHUNKS_H
#define __REQUEST_MISSING_CHUNKS_H
#include "client/hiface.h"
#include "shared/types/geom.h"
#include "shared/types/hash.h"

void request_missing_chunks_init(void);
void request_missing_chunks(struct hiface *hif, const struct rectangle *r);
#endif
