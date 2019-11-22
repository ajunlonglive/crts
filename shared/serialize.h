#ifndef __SERIALIZE_H
#define __SERIALIZE_H
#include "update.h"
#include "world.h"
#include "action.h"

int pack_ent(struct ent *e, char *buf);
int unpack_ent(struct ent *e, const char *buf);
int pack_action(struct action *a, char *buf);
int unpack_action(struct action *a, const char *buf);
int pack_update(const struct update *ud, char *buf);
int unpack_update(struct update *ud, const char *buf);
#endif
