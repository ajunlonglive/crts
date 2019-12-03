#ifndef __SERIALIZE_SIM_H
#define __SERIALIZE_SIM_H
#include "sim/world.h"
#include "sim/action.h"

void log_bytes(const char *bytes, size_t n);

size_t pack_ent(struct ent *e, char *buf);
size_t unpack_ent(struct ent *e, const char *buf);

size_t pack_action(struct action *a, char *buf);
size_t unpack_action(struct action *a, const char *buf);
#endif
