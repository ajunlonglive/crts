#include <stdlib.h>

#include "client/cfg/keymap.h"
#include "client/hiface.h"

struct hiface *
hiface_init(struct simulation *sim)
{
	struct hiface *hf = calloc(1, sizeof(struct hiface));

	hf->sim = sim;
	hf->im = im_normal;
	hf->km = parse_keymap("cfg/keymap.ini");
	hf->next_act.range.r = 3;

	return hf;
}

long
hiface_get_num(struct hiface *hif, long def)
{
	return (hif->num.len <= 0) ? def : strtol(hif->num.buf, NULL, 10);
}
