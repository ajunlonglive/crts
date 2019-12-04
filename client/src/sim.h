#ifndef __CLIENT_SIM_H
#define __CLIENT_SIM_H
struct simulation {
	struct queue *outbound;
	struct queue *inbound;
	struct world *w;
};
#endif
