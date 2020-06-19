#ifndef CLIENT_UI_OPENGL_OBJ_LOADER_H
#define CLIENT_UI_OPENGL_OBJ_LOADER_H

#include <stdbool.h>

#include "shared/types/darr.h"

/* 3 for position, 3 for normal */
typedef float vertex_elem[6];

void obj_loader_setup(void);
bool obj_load(char *filename, struct darr *verts, struct darr *indices, float scale);
#endif
