#ifndef __SERIALIZE_SERVER_MESSAGE_H
#define __SERIALIZE_SERVER_MESSAGE_H

#include "shared/messaging/server_message.h"

size_t pack_sm(const struct server_message *ud, char *buf);
size_t unpack_sm(struct server_message *ud, const char *buf);

size_t pack_sm_ent(const struct sm_ent *eu, char *buf);
size_t unpack_sm_ent(struct sm_ent *eu, const char *buf);

size_t pack_sm_chunk(const struct sm_chunk *eu, char *buf);
size_t unpack_sm_chunk(struct sm_chunk *eu, const char *buf);

size_t pack_sm_action(const struct sm_action *eu, char *buf);
size_t unpack_sm_action(struct sm_action *eu, const char *buf);

size_t pack_sm_world_info(const struct sm_world_info *eu, char *buf);
size_t unpack_sm_world_info(struct sm_world_info *eu, const char *buf);

size_t pack_sm_rem_action(const struct sm_rem_action *eu, char *buf);
size_t unpack_sm_rem_action(struct sm_rem_action *eu, const char *buf);
#endif
