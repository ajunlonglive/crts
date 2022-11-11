#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/chunks.h"
#include "client/ui/gl/render/selection.h"
#include "client/ui/gl/shader.h"
#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "tracy.h"

typedef float highlight_block[8][2][3];

static struct darr selection_data = { 0 };
static struct darr draw_counts = { 0 };
static struct darr draw_indices = { 0 };
static struct darr draw_baseverts = { 0 };

static const uint32_t sel_indices[] = {
	1, 3, 0,
	3, 2, 0,
	1, 5, 3,
	5, 7, 3,
	1, 5, 0,
	5, 4, 0,
	2, 6, 0,
	6, 4, 0,
	3, 7, 2,
	7, 6, 2
};

size_t sel_indices_len = 30;
void *null_pointer = 0;

enum sel_uniform {
	su_pulse = UNIFORM_START_RP_FINAL,
};

struct shader sel_shader;

bool
render_world_setup_selection(void)
{
	struct shader_spec sel_spec = {
		.src = {
			[rp_final] = {
				{ "selection.vert", GL_VERTEX_SHADER },
				{ "basic.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = { [rp_final] = { { su_pulse, "pulse" } } },
		.attribute = { { { 3, GL_FLOAT, bt_vbo }, { 3, GL_FLOAT, bt_vbo } } },
		.static_data = {
			{ sel_indices, sizeof(uint32_t) * sel_indices_len, bt_ebo },
		},
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj
						| 1 << duf_clip_plane),
		}
	};

	if (!shader_create(&sel_spec, &sel_shader)) {
		return false;
	}

	darr_init(&selection_data, sizeof(highlight_block));
	darr_init(&draw_counts, sizeof(GLsizei));
	darr_init(&draw_indices, sizeof(GLvoid *));
	darr_init(&draw_baseverts, sizeof(GLint));

	return true;
}

#if 0
static float
get_avg_height_at_center(struct point *curs)
{
	chunk_mesh *ck;
	struct point np = nearest_chunk(curs);

	if (!(ck = hdarr_get(chunk_meshes, &np))) {
		return 0.0f;
	}

	np = point_sub(curs, &np);

	float sum = 0.0f;
	uint32_t i;
	for (i = 0; i < 4; ++i) {
		int32_t x = np.x + i % 2;
		int32_t y = np.y + i / 2;
		int32_t ii = y * MESH_DIM + x;
		sum += (*ck)[ii].pos[1];
	}

	return sum / 4.0f;
}
#endif

static void
setup_hightlight_block(struct gl_ui_ctx *ctx, float h, vec4 clr, struct point *curs)
{
	highlight_block sel = { 0 };
	uint8_t i;
	struct point np;
	chunk_mesh *ck;
	int x, y, ii;
	float height;

	np = nearest_chunk(curs);

	if ((ck = hdarr_get(&ctx->chunk_meshes, &np))) {
		np = point_sub(curs, &np);
	}

	for (i = 0; i < 8; ++i) {
		x = np.x + i % 2;
		y = np.y + (i % 4) / 2;
		ii = y * MESH_DIM + x;

		height = h * (1 - (i / 4));

		if (ck) {
			height += (*ck)[ii].pos[1];
		}

		sel[i][0][0] = curs->x + (i % 2) - 0.5;
		sel[i][0][1] =  height;
		sel[i][0][2] = curs->y + ((i % 4) / 2) - 0.5;

		sel[i][1][0] = clr[0];
		sel[i][1][1] = clr[1];
		sel[i][1][2] = clr[2];
	}

	darr_push(&draw_counts, &sel_indices_len);
	darr_push(&draw_indices, &null_pointer);

	size_t basevert = darr_len(&selection_data) * 8;
	darr_push(&draw_baseverts, &basevert);

	darr_push(&selection_data, sel);
}

void
render_selection_setup_frame(struct client *cli, struct gl_ui_ctx *ctx)
{
	TracyCZoneAutoS;

	darr_clear(&selection_data);
	darr_clear(&draw_counts);
	darr_clear(&draw_indices);
	darr_clear(&draw_baseverts);

	vec4 clr = { 0, 1, 1, 1 };
	setup_hightlight_block(ctx, 1.0, clr, &cli->cursor);

	glBindBuffer(GL_ARRAY_BUFFER, sel_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(highlight_block) * darr_len(&selection_data),
		darr_raw_memory(&selection_data), GL_DYNAMIC_DRAW);

	TracyCZoneAutoE;
}

void
render_selection(struct client *cli, struct gl_ui_ctx *ctx)
{
	assert(ctx->pass == rp_final);

	glUseProgram(sel_shader.id[rp_final]);
	shader_check_def_uni(&sel_shader, ctx);

	glBindVertexArray(sel_shader.vao[rp_final][0]);

	float pulse = (sinf(ctx->pulse_ms / 100.0f) + 1.0f) / 2.0f;
	glUniform1fv(sel_shader.uniform[rp_final][su_pulse], 1, &pulse);

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		darr_raw_memory(&draw_counts),
		GL_UNSIGNED_INT,
		darr_raw_memory(&draw_indices),
		darr_len(&selection_data),
		darr_raw_memory(&draw_baseverts));
}
