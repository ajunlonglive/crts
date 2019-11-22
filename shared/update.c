#include "world.h"
#include "update.h"

struct update *ent_update_init(struct ent *e)
{
	struct ent_update *eu = malloc(sizeof(struct ent_update));
	struct update *u = malloc(sizeof(struct update));

	if (e != NULL) {
		eu->id = e->id;
		eu->pos = e->pos;
	}

	u->type = update_type_ent;
	u->update = eu;

	return u;
}

void update_destroy(struct update *ud)
{
	free(ud->update);
	free(ud);
}
