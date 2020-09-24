#ifndef CLIENT_NET_H
#define CLIENT_NET_H

struct c_simulation;

struct net_ctx *net_init(const char *ipv4addr, struct c_simulation *sim);
void net_set_outbound_id(long id);
void check_add_server_cx(struct net_ctx *nx);
#endif
