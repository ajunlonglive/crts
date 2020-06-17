#include "posix.h"

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/obj_loader.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/winutil.h"
#include "shared/util/log.h"

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos, positions, types, colors;
	size_t triangle_count, vert_count;
} s_ent = { 0 };

bool
render_world_setup_ents(void)
{
	bool init = false;

	struct shader_src src[] = {
		{ "ents.vert", GL_VERTEX_SHADER },
		{ "world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_ent.id)) {
		return false;
	}

	s_ent.view      = glGetUniformLocation(s_ent.id, "view");
	s_ent.proj      = glGetUniformLocation(s_ent.id, "proj");
	s_ent.positions = glGetUniformLocation(s_ent.id, "positions");
	s_ent.types     = glGetUniformLocation(s_ent.id, "types");
	s_ent.colors    = glGetUniformLocation(s_ent.id, "colors");
	s_ent.view_pos  = glGetUniformLocation(s_ent.id, "view_pos");

	glUseProgram(s_ent.id);

	glGenVertexArrays(1, &s_ent.vao);
	glGenBuffers(1, &s_ent.vbo);
	glGenBuffers(1, &s_ent.ebo);

	struct darr *obj_verts   = darr_init(sizeof(vertex_elem));
	struct darr *obj_indices = darr_init(sizeof(uint32_t));
	//obj_load("assets/deer.obj", obj_verts, obj_indices, 0.0016f);
	//obj_load("assets/tree.obj", obj_verts, obj_indices, 0.8f);
	if (!obj_load("cube.obj", obj_verts, obj_indices, 1.0f)) {
		LOG_W("failed to load asset");
		goto free_exit;
	}

	s_ent.triangle_count = darr_len(obj_indices);
	s_ent.vert_count = darr_len(obj_verts);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ent.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * darr_len(obj_indices),
		darr_raw_memory(obj_indices), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_elem) * darr_len(obj_verts),
		darr_raw_memory(obj_verts), GL_STATIC_DRAW);

	glBindVertexArray(s_ent.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_elem),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_elem),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// send colors
	glUniform4fv(s_ent.colors, extended_ent_type_count, (float *)colors.ent);

	init = true;

free_exit:
	darr_destroy(obj_verts);
	darr_destroy(obj_indices);

	return init;
}

void
render_ents(struct hiface *hf, struct opengl_ui_ctx *ctx, mat4 mview)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(hf->sim->w->ents));
	size_t i, j = 0, len = hdarr_len(hf->sim->w->ents);

	float positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	size_t skipped = 0, rendered = 0;

	glUseProgram(s_ent.id);
	glBindVertexArray(s_ent.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ent.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)ctx->mproj);
		glUniformMatrix4fv(s_ent.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_ent.view_pos, 1, cam.pos);
	}

	for (i = 0; i < len; ++i) {
		if (!point_in_rect(&emem[i].pos, &ctx->ref)) {
			++skipped;
			continue;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(hf->sim->w->chunks->hd, &p);

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;
		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			if (ck->tiles[p.x][p.y] <= tile_water) {
				positions[(j * 3) + 2] = -2.0;
			} else {
				positions[(j * 3) + 2] = 0.5 + ck->heights[p.x][p.y];
			}
		}

		if ((types[j] = emem[i].type) == et_worker) {
			if (emem[i].alignment == hf->sim->assigned_motivator) {
				types[j] = et_elf_friend;
			} else {
				types[j] = et_elf_foe;
			}
		}

		if (++j >= 256) {
			glUniform1uiv(s_ent.types, j, types);
			glUniform3fv(s_ent.positions, j, positions);
			//glDrawArraysInstanced(GL_POINTS, 0, 540, j);
			glDrawElementsInstanced(GL_TRIANGLES, s_ent.triangle_count, GL_UNSIGNED_INT, 0, j);
			j = 0;
		}

		++rendered;
	}

	glUniform1uiv(s_ent.types, j, types);
	glUniform3fv(s_ent.positions, j, positions);
	//glDrawArraysInstanced(GL_POINTS, 0, 540, j);
	glDrawElementsInstanced(GL_TRIANGLES, s_ent.triangle_count, GL_UNSIGNED_INT, 0, j);
}
