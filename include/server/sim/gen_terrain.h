#ifndef SERVER_SIM_GEN_TERRAIN_H
#define SERVER_SIM_GEN_TERRAIN_H

#include "shared/sim/chunk.h"

void gen_terrain(struct chunks *chunks, uint32_t width, uint32_t height, uint32_t iterations);
#endif
