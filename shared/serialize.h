#ifndef __SERIALIZE_H
#define __SERIALIZE_H
#include "world.h"
#include "action.h"

int pack_ent(struct ent *e, char *buf);
int unpack_ent(struct ent *e, const char *buf);
int pack_action(struct action *a, char *buf);
int unpack_action(struct action *a, const char *buf);
#endif
