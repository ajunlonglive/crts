#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/sim/do_action/dismount.h"
#include "shared/types/result.h"

enum result
do_action_dismount(struct simulation *sim, struct ent *e, struct sim_action *sa)
{
	return rs_done;
}
