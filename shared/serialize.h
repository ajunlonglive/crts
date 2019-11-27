#ifndef __SERIALIZE_H
#define __SERIALIZE_H
#include "update.h"
#include "world.h"
#include "action.h"

void log_bytes(const char *bytes, size_t n);

size_t pack_ent(struct ent *e, char *buf);
size_t unpack_ent(struct ent *e, const char *buf);

size_t pack_action(struct action *a, char *buf);
size_t unpack_action(struct action *a, const char *buf);

size_t pack_update(const struct update *ud, char *buf);
size_t unpack_update(struct update *ud, const char *buf);

size_t pack_ent_update(const struct ent_update *eu, char *buf);
size_t unpack_ent_update(struct ent_update *eu, const char *buf);

size_t pack_action_update(const struct action_update *eu, char *buf);
size_t unpack_action_update(struct action_update *eu, const char *buf);

#endif
