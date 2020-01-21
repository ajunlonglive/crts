#include <stdlib.h>
#include "hiface.h"
#include "cfg/keymap.h"

struct hiface *
hiface_init(struct simulation *sim)
{
	struct hiface *hf = calloc(1, sizeof(struct hiface));

	hf->sim = sim;
	hf->im = im_normal;
	hf->km = parse_keymap("defcfg/keymap.ini");

	return hf;
}
