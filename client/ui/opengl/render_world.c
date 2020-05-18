#include <math.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render_world.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"

static struct {
	uint32_t id;
	uint32_t vao, vbo;
	uint32_t view, proj, view_pos, positions, types;
	struct hash *h;
} s_ent = { 0 };

static struct {
	uint32_t id;
	uint32_t view, proj, view_pos;
} s_chunk = { 0 };

static bool
render_world_setup_ents(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/ents.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_ent.id)) {
		return false;
	}

	s_ent.view      = glGetUniformLocation(s_ent.id, "view");
	s_ent.proj      = glGetUniformLocation(s_ent.id, "proj");
	s_ent.positions = glGetUniformLocation(s_ent.id, "positions");
	s_ent.types     = glGetUniformLocation(s_ent.id, "types");
	s_ent.view_pos  = glGetUniformLocation(s_ent.id, "view_pos");

	s_ent.h = hash_init(2048, 1, sizeof(struct point));

	glGenVertexArrays(1, &s_ent.vao);
	glGenBuffers(1, &s_ent.vbo);

	glBindVertexArray(s_ent.vao);
	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * solid_cube.len,
		solid_cube.verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return true;
}

bool
render_world_setup_chunks(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/chunks.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_chunk.id)) {
		return false;
	}

	s_chunk.view      = glGetUniformLocation(s_chunk.id, "view");
	s_chunk.proj      = glGetUniformLocation(s_chunk.id, "proj");
	s_chunk.view_pos  = glGetUniformLocation(s_chunk.id, "view_pos");

	return true;
}

bool
render_world_setup(char *graphics_path)
{
	return render_world_setup_ents()
	       && render_world_setup_chunks()
	       && color_cfg(graphics_path);
}

void
render_world_teardown(void)
{
	hash_destroy(s_ent.h);
}

void
update_world_viewport(mat4 mproj)
{
	glUseProgram(s_ent.id);
	glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)mproj);

	/*
	   glUseProgram(s_chunk.id);
	   glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)mproj);
	 */
}

/*
   static void
   render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
   {
        int ipos[3] = { 0 };

        struct chunk *ck;
        struct point sp = nearest_chunk(&ctx->ref.pos);
        int spy = sp.y,
            endx = ctx->ref.pos.x + ctx->ref.width,
            endy = ctx->ref.pos.y + ctx->ref.height;

        for (; sp.x < endx; sp.x += CHUNK_SIZE) {
                for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
                        if ((ck = hdarr_get(cnks->hd, &sp))) {
                                ipos[0] = ck->pos.x;
                                ipos[1] = ck->pos.y;

                                glUniform3iv(ctx->chunks.uni.positions, 1, ipos);
                                glUniform1uiv(ctx->chunks.uni.types, 256,
                                        (uint32_t *)&ck->tiles);
                        }
                }
        }
   }
 */

static void
render_ents(struct hdarr *ents, struct hdarr *cnks, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(ents));
	size_t i, j, len = hdarr_len(ents);

	hash_clear(s_ent.h);

	int32_t positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	const size_t *st;

	glUseProgram(s_ent.id);
	glBindVertexArray(s_ent.vao);

	for (i = 0, j = 0; i < len; ++i, ++j) {
		if (i >= 256) {
			glUniform1uiv(s_ent.types, 256, types);
			glUniform3iv(s_ent.positions, 256, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256);
			j = 0;
		}

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;

		if ((st = hash_get(s_ent.h, &emem[i].pos))) {
			positions[(j * 3) + 2] = *st + 1;
			hash_set(s_ent.h, &emem[i].pos, *st + 1);
		} else {
			positions[(j * 3) + 2] = 0;
			hash_set(s_ent.h, &emem[i].pos, 0);
		}

		types[j] = emem[i].type;
	}

	glUniform1uiv(s_ent.types, 256, types);
	glUniform3iv(s_ent.positions, 256, positions);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, len % 256);
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	if (cam.changed) {
		mat4 mview;

		ctx->ref.pos = hf->view;

		float w = cam.pos[1] * (float)ctx->width / (float)ctx->height / 2;
		float h = cam.pos[1] * tanf(FOV / 2) * 2;

		ctx->ref.width = w;
		ctx->ref.height = h;

		cam.pos[0] = ctx->ref.pos.x + w * 0.5;
		cam.pos[2] = ctx->ref.pos.y + h * 0.5;

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);

		glUseProgram(s_ent.id);
		glUniformMatrix4fv(s_ent.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_ent.view_pos, 1, cam.pos);

		cam.changed = false;
	}

	//render_chunks(hf->sim->w->chunks, ctx);

	render_ents(hf->sim->w->ents, hf->sim->w->chunks->hd, ctx);
}
