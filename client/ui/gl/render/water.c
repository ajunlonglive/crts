#include "posix.h"

#include <assert.h>
#include <math.h>

#include "client/client.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/water.h"
#include "client/ui/gl/shader.h"
#include "shared/ui/gl/util.h"
#include "shared/util/log.h"

static struct shader water_shader;

enum {
	su_reflect_tex = UNIFORM_START_RP_FINAL,
	su_refract_tex,
	su_ripple_tex,
	su_pulse,
};

static struct { int32_t ripple; } textures;

bool
render_world_setup_water(struct water_fx *wfx)
{
	/* refract */
	glGenFramebuffers(1, &wfx->refract_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx->refract_fb);
	fb_attach_color(wfx->refract_w, wfx->refract_h, &wfx->refract_tex, &wfx->refract_db);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* reflect */
	glGenFramebuffers(1, &wfx->reflect_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx->reflect_fb);
	fb_attach_color(wfx->reflect_w, wfx->reflect_h, &wfx->reflect_tex, &wfx->reflect_db);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	uint32_t indices[] = { 0, 1, 2, 3, 0, 2 };

	struct shader_spec spec = {
		.src = {
			[rp_final] =  {
				{ "water.vert", GL_VERTEX_SHADER },
				{ "water.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = {
			[rp_final] = {
				{ su_reflect_tex, "reflect_tex" },
				{ su_refract_tex, "refract_tex" },
				{ su_ripple_tex, "ripple_tex" },
				{ su_pulse, "pulse" },
			}
		},
		.attribute = { { { 3, GL_FLOAT, bt_vbo } } },
		.static_data = { { indices, sizeof(uint32_t) * 6, bt_ebo } },
		.uniform_blacklist = {
			[rp_final] = 1 << duf_light_space
				     | 1 << duf_clip_plane
				     | 1 << duf_light_pos
				     | 1 << duf_view_pos,
		}
	};

	if (!shader_create(&spec, &water_shader)) {
		return false;
	}

	glUseProgram(water_shader.id[rp_final]);
	glUniform1i(water_shader.uniform[rp_final][su_reflect_tex], 0);
	glUniform1i(water_shader.uniform[rp_final][su_refract_tex], 1);
	glUniform1i(water_shader.uniform[rp_final][su_ripple_tex], 2);

	if ((textures.ripple = load_tex("water.tga", GL_REPEAT, GL_LINEAR)) == -1) {
		return false;
	}

	return true;
}

#define WATER_LVL 0.0f

void
render_water_setup_frame(struct client *cli, struct gl_ui_ctx *ctx)
{
	if (!ctx->ref_changed) {
		return;
	}

	float quad[] = {
		cli->ref.rect.p[0].x, WATER_LVL, cli->ref.rect.p[0].y,
		cli->ref.rect.p[1].x, WATER_LVL, cli->ref.rect.p[1].y,
		cli->ref.rect.p[2].x, WATER_LVL, cli->ref.rect.p[2].y,
		cli->ref.rect.p[3].x, WATER_LVL, cli->ref.rect.p[3].y,
	};

	glBindBuffer(GL_ARRAY_BUFFER, water_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, quad, GL_DYNAMIC_DRAW);

}

void
render_water(struct gl_ui_ctx *ctx, struct water_fx *wfx)
{
	glUseProgram(water_shader.id[rp_final]);
	shader_check_def_uni(&water_shader, ctx);

	float pulse = sinf(ctx->pulse_ms / 10000.0f);
	glUniform1fv(water_shader.uniform[rp_final][su_pulse], 1, &pulse);

	glBindVertexArray(water_shader.vao[rp_final][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wfx->reflect_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wfx->refract_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures.ripple);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
}
