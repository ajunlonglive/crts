#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/render/selection.h"
#include "client/ui/opengl/obj_loader.h"
#include "client/ui/opengl/globals.h"

static struct hdarr *chunk_meshes;

bool
render_world_setup(void)
{
	obj_loader_setup();

	return render_world_setup_ents()
	       && render_world_setup_chunks(&chunk_meshes)
	       && render_world_setup_selection();
}

void
render_world_teardown(void)
{
	hdarr_destroy(chunk_meshes);
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	float w, h;
	static struct rectangle oref = { 0 };
	bool ref_changed = false;
	static bool reset_chunks = false;

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		ctx->ref.pos = hf->view;

		if (!cam.unlocked) {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height * 0.48;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w * 0.5;
			cam.pos[2] = ctx->ref.pos.y + h * 0.5;
			ctx->ref.width = w;
			ctx->ref.height = h;
		}

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);
		cam.changed = true;

		if ((ref_changed = memcmp(&oref, &ctx->ref, sizeof(struct rectangle)))) {
			oref = ctx->ref;
		}
	}

	/* ents */
	render_ents(hf, ctx, mview);

	/* selection */
	render_selection(hf, ctx, chunk_meshes, mview, reset_chunks);

	/* chunks */
	reset_chunks = render_chunks(hf, ctx, chunk_meshes, mview,
		ref_changed);

	/* last usage of cam.changed */
	cam.changed = false;
}
