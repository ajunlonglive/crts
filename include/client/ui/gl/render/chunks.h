#ifndef CLIENT_UI_OPENGL_RENDER_CHUNKS_H
#define CLIENT_UI_OPENGL_RENDER_CHUNKS_H

#include "client/ui/gl/ui.h"
#include "shared/sim/chunk.h"

#define MESH_DIM (CHUNK_SIZE + 1)

struct chunk_info {
	float pos[3];
	float norm[3];
	float type;
};

typedef struct chunk_info chunk_mesh[MESH_DIM * MESH_DIM];

bool render_world_setup_chunks(struct hdarr *chunk_meshes);
void render_chunks_setup_frame(struct client *cli, struct gl_ui_ctx *ctx,
	struct hdarr *cms);
void render_chunks(struct client *cli, struct gl_ui_ctx *ctx, struct hdarr *cms,
	bool render_bottoms);
#endif
