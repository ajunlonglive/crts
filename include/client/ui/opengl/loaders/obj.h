#ifndef CLIENT_UI_OPENGL_OBJ_LOADER_H
#define CLIENT_UI_OPENGL_OBJ_LOADER_H

#include <stdbool.h>

#include "shared/types/darr.h"

void obj_loader_setup(void);
bool obj_load(char *filename, struct darr *verts, struct darr *norms,
	struct darr *indices, float scale);
#endif
