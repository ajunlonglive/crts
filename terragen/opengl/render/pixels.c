#include "posix.h"

#include <math.h>
#include <stdlib.h>

#include "shared/opengl/shader.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/opengl/render/pixels.h"
#include "terragen/opengl/ui.h"

struct shader pixels_shader;

enum render_pixels_uniform {
	rpu_tex = UNIFORM_START_RP_FINAL,
};

typedef uint8_t pix[4];
pix *img;
size_t img_size;
uint32_t texture;

bool
render_pixels_setup(struct ui_ctx *ctx)
{
	uint32_t indices[] = { 0, 1, 2, 1, 2, 3 };
	float quad[] = {
		-1, 1, 0.0f, 1.0f,
		1, 1, 1.0f, 1.0f,
		-1, -1, 0.0f, 0.0f,
		1, -1, 1.0f, 0.0f,
	};

	struct shader_spec spec = {
		.src = {
			[rp_final] = {
				{ "terragen_pixels.vert", GL_VERTEX_SHADER },
				{ "terragen_pixels.frag", GL_FRAGMENT_SHADER },
			},
		},
		.uniform = { [rp_final] = { { rpu_tex, "tex" } } },
		.attribute = {
			{ { 2, GL_FLOAT, bt_vbo, true }, { 2, GL_FLOAT, bt_vbo } }
		},
		.uniform_blacklist = { [rp_final] = 0xffff },
		.static_data = { { indices, sizeof(uint32_t) * 6, bt_ebo },
				 { quad, sizeof(float) * 16, bt_vbo } },
		.interleaved = true
	};

	if (!shader_create(&spec, &pixels_shader)) {
		return false;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUseProgram(pixels_shader.id[rp_final]);
	glUniform1i(pixels_shader.uniform[rp_final][rpu_tex], 0);

	return true;
}

void
render_pixels_setup_frame(struct ui_ctx *ctx)
{
	uint32_t i;
	size_t size = ctx->ctx.a;

	if (size != img_size) {
		img = realloc(img, size * sizeof(pix));
		img_size = size;
	}

	float max_elev = -INFINITY;
	for (i = 0; i < size; ++i) {
		struct terrain_pixel *tp = &ctx->ctx.terra.heightmap[i];

		if (tp->elev > max_elev) {
			max_elev = tp->elev;
		}
	}

	for (i = 0; i < size; ++i) {
		struct terrain_pixel *tp = &ctx->ctx.terra.heightmap[i];
		bool land = tp->elev > 0;
		float nh = land ? tp->elev : -tp->elev;

		if (nh > max_elev) {
			nh = max_elev;
		}

		nh /= max_elev;

		img[i][0] = img[i][1] = img[i][2] = 0;

		if (land) {
			img[i][1] = floorf(nh * 128) + 20;
		} else {
			img[i][2] = floorf(nh * 100) + 20;
		}

#define MW 1.0f
#define MS 1.0f

		float wtr;

		wtr = tp->e.d > MW ? 1.0 : tp->e.d / MW;
		img[i][2] += floorf(wtr * 128);

		/* wtr = tp->e.s > MS ? 1.0 : tp->e.s / MS; */
		/* img[i][0] = floorf(wtr * 100); */

		img[i][3] = tp->filled ? floorf(255 * ctx->heightmap_opacity) : 0;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctx->ctx.l,
		ctx->ctx.l, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
}

void
render_pixels(struct ui_ctx *ctx)
{
	glUseProgram(pixels_shader.id[rp_final]);
	glBindVertexArray(pixels_shader.vao[rp_final][0]);

	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
