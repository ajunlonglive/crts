#include "action.h"
#include "world.h"
#include "update.h"

static struct update poke_update = {
	.type = update_type_poke,
	.update = NULL
};

struct update *ent_update_init(struct ent *e)
{
	struct ent_update *eu = malloc(sizeof(struct ent_update));
	struct update *u = malloc(sizeof(struct update));

	if (e != NULL) {
		eu->id = e->id;
		eu->pos = e->pos;
		eu->alignment = e->alignment->max;
	}

	u->type = update_type_ent;
	u->update = eu;

	return u;
}

struct update *poke_update_init()
{
	return &poke_update;
}

struct update *action_update_init(struct action *a)
{
	struct update *u = malloc(sizeof(struct update));
	struct action_update *au = malloc(sizeof(struct action_update));

	if (a != NULL) {
		au->type = a->type;
		au->range = a->range;
	}

	u->type = update_type_action;
	u->update = au;

	return u;
}

void update_destroy(struct update *ud)
{
	switch (ud->type) {
	case update_type_poke:
		break;
	default:
		if (ud->update != NULL)
			free(ud->update);

		free(ud);
		break;
	}
}
