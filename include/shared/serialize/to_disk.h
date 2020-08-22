#ifndef SHARED_SERALIZE_TO_DISK
#define SHARED_SERALIZE_TO_DISK
#include <stdio.h>

#include "shared/sim/chunk.h"

void write_chunks(FILE *f, struct chunks *chunks);
void read_chunks(FILE *f, struct chunks *chunks);
#endif
