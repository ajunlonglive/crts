#ifndef CLIENT_REQUEST_MISSING_CHUNKS_H
#define CLIENT_REQUEST_MISSING_CHUNKS_H

#include "client/hiface.h"
#include "shared/net/net_ctx.h"
#include "shared/types/geom.h"

void request_missing_chunks_init(void);
void request_missing_chunks(struct hiface *hif, const struct rectangle *r, struct net_ctx *nx);
#endif
