#ifndef CLIENT_REQUEST_MISSING_CHUNKS_H
#define CLIENT_REQUEST_MISSING_CHUNKS_H

#include "client/client.h"
#include "shared/types/geom.h"

void request_missing_chunks_init(void);
void request_missing_chunks(struct client *cli);
#endif
