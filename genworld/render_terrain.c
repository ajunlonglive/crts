#include "posix.h"

#include "genworld/gl.h"
#include "genworld/render_terrain.h"
#include "shared/math/rand.h"
#include "shared/opengl/shader.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct shader terrain_shader;

enum render_terrain_uniform {
	rtu_proj,
};

struct darr *tris;

typedef float dat[6];

bool
render_terrain_init(struct ui_ctx *ctx)
{
	struct shader_spec spec = {
		.src = {
			[rp_final] = {
				{ "terrain.vert", GL_VERTEX_SHADER },
				{ "terrain.frag", GL_FRAGMENT_SHADER },
			},
		},
		.uniform = { [rp_final] = { { rtu_proj, "proj" } } },
		.attribute = {
			{ { 3, GL_FLOAT, bt_vbo, true }, { 3, GL_FLOAT, bt_vbo } }
		},
		.uniform_blacklist = { [rp_final] = 0xffff },
		.interleaved = true
	};

	if (!shader_create(&spec, &terrain_shader)) {
		return false;
	}

	tris = darr_init(sizeof(dat));

	return true;
}

static void
regen_proj_matrix(struct ui_ctx *ctx)
{
	mat4 ortho, mscale, proj;

	gen_fake_ortho_mat4(0.0, ctx->win.width, 0.0, ctx->win.height, ortho);

	vec4 scale = {
		ctx->win.width / (float)ctx->gt.terra.opts.width,
		ctx->win.height / (float)ctx->gt.terra.opts.height,
		0.0f,
		0.0f
	};

	gen_scale_mat4(scale, mscale);

	mat4_mult_mat4(ortho, mscale, proj);

	glUniformMatrix4fv(terrain_shader.uniform[rp_final][rtu_proj], 1,
		GL_TRUE, (float *)proj);
}


enum line_clr {
	lc_blank,
	lc_fault_mtn,
	lc_fault_val,
	lc_land,
	lc_sea
};

static enum line_clr
line_clr(const struct terrain_vertex *tv)
{
	if (tv) {
		if (tv->fault) {
			return tv->elev > 0 ? lc_fault_mtn : lc_fault_val;
		} else {
			return tv->elev > 0 ? lc_land : lc_sea;
		}
	} else {
		return lc_blank;
	};
}

void
render_terrain_setup(struct ui_ctx *ctx)
{
	darr_clear(tris);

	static const vec4 clrs[] = {
		[lc_blank] = { 0.2, 0.2, 0.2 },
		[lc_fault_val] = { 0.4, 0.1, 0.7 },
		[lc_fault_mtn] = { 0.7, 0.2, 0.2 },
		[lc_land] = { 0.2, 0.8, 0.2 },
		[lc_sea] = { 0.0, 0.1, 0.3 },
	};

	uint32_t i;
	for (i = 0; i < hdarr_len(ctx->gt.tg.tris); ++i) {
		const struct tg_tri *t = darr_try_get(hdarr_darr(ctx->gt.tg.tris), i);

		if (!t) {
			continue;
		}

		const size_t *id[] = {
			hdarr_get_i(ctx->gt.terra.tdat, t->a),
			hdarr_get_i(ctx->gt.terra.tdat, t->b),
			hdarr_get_i(ctx->gt.terra.tdat, t->c),
		};
		const struct terrain_vertex *tv[] = {
			id[0] ? darr_try_get(hdarr_darr(ctx->gt.terra.tdat), *id[0]) : NULL,
			id[1] ? darr_try_get(hdarr_darr(ctx->gt.terra.tdat), *id[1]) : NULL,
			id[2] ? darr_try_get(hdarr_darr(ctx->gt.terra.tdat), *id[2]) : NULL,
		};

		enum line_clr f[3] = { line_clr(tv[0]), line_clr(tv[1]), line_clr(tv[2]) };

		dat pdat[3] = {
			{ t->a->x, t->a->y, 0.0f,
			  clrs[f[0]][0], clrs[f[0]][1], clrs[f[0]][2], },
			{ t->b->x, t->b->y, 0.0f,
			  clrs[f[1]][0], clrs[f[1]][1], clrs[f[1]][2], },
			{ t->c->x, t->c->y, 0.0f,
			  clrs[f[2]][0], clrs[f[2]][1], clrs[f[2]][2], },
		};

		darr_push(tris, pdat[0]);
		darr_push(tris, pdat[1]);
		darr_push(tris, pdat[1]);
		darr_push(tris, pdat[2]);
		darr_push(tris, pdat[2]);
		darr_push(tris, pdat[0]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, terrain_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(tris),
		darr_raw_memory(tris), GL_DYNAMIC_DRAW);
}

void
render_terrain(struct ui_ctx *ctx)
{
	glUseProgram(terrain_shader.id[rp_final]);
	glBindVertexArray(terrain_shader.vao[rp_final][0]);

	if (ctx->win.resized) {
		regen_proj_matrix(ctx);
	}

	glLineWidth(2);
	glDrawArrays(GL_LINES,
		0,
		darr_len(tris));
}
