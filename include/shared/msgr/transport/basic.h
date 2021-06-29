#ifndef SHARED_MSGR_TRANSPORT_BASIC_H
#define SHARED_MSGR_TRANSPORT_BASIC_H

#include "shared/msgr/msgr.h"
#include "shared/types/ring_buffer.h"

void msgr_transport_init_basic_pipe(struct msgr *a, struct msgr *b);
#endif
