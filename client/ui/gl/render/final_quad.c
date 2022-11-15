#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/final_quad.h"
#include "client/ui/gl/shader.h"
#include "shared/sim/tiles.h"
#include "shared/ui/gl/util.h"
#include "shared/util/log.h"

static struct shader final_quad_shader;

enum {
	su_world_tex = UNIFORM_START_RP_FINAL,
	su_depth_tex,
	su_focus,
	su_inv_view,
	su_inv_proj,
};


bool
render_world_setup_final_quad(struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx)
{
	glGenFramebuffers(1, &fqfx->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fqfx->fb);
	fqfx->fb_tex = fb_attach_color(ctx->win->px_width, ctx->win->px_height);
	fqfx->fb_depth = fb_attach_dtex(ctx->win->px_width, ctx->win->px_height);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, fqfx->fb_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const uint32_t indices[] = { 3, 1, 2, 2, 1, 0, };
	const float quad[] = {
		-1, 1, 0.0f, 1.0f,
		1, 1, 1.0f, 1.0f,
		-1, -1, 0.0f, 0.0f,
		1, -1, 1.0f, 0.0f,
	};

	struct shader_spec spec = {
		.src = {
			[rp_final] =  {
				{ "final_quad.vert", GL_VERTEX_SHADER },
				{ "final_quad.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = {
			[rp_final] = {
				{ su_world_tex, "world_tex" },
				{ su_depth_tex, "depth_tex" },
				{ su_focus, "focus" },
				{ su_inv_view, "inv_view" },
				{ su_inv_proj, "inv_proj" },
			}
		},
		.attribute = {
			{ { 2, GL_FLOAT, bt_vbo, true }, { 2, GL_FLOAT, bt_vbo } }
		},
		.static_data = { { indices, sizeof(uint32_t) * 6, bt_ebo },
				 { quad, sizeof(float) * 16, bt_vbo } },
		.uniform_blacklist = {
			[rp_final] = 1 << duf_light_space
				     | 1 << duf_clip_plane
				     | 1 << duf_light_pos
				     | 1 << duf_view_pos
				     | 1 << duf_viewproj,
		},
		.interleaved = true
	};

	if (!shader_create(&spec, &final_quad_shader)) {
		return false;
	}

	glUseProgram(final_quad_shader.id[rp_final]);
	glUniform1i(final_quad_shader.uniform[rp_final][su_world_tex], 0);
	glUniform1i(final_quad_shader.uniform[rp_final][su_depth_tex], 1);

	return true;
}

void
render_final_quad_setup_frame(struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx)
{
	if (!ctx->win->resized) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, fqfx->fb_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		ctx->win->px_width, ctx->win->px_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, fqfx->fb_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		ctx->win->px_width, ctx->win->px_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

void
render_final_quad(struct client *cli, struct gl_ui_ctx *ctx, struct final_quad_fx *fqfx)
{
	glUseProgram(final_quad_shader.id[rp_final]);
	shader_check_def_uni(&final_quad_shader, ctx);

	vec3 cursor = { cli->cursorf.x, get_height_at(&cli->world->chunks, &cli->cursor), cli->cursorf.y };

	glUniform3fv(final_quad_shader.uniform[rp_final][su_focus], 1, cursor);
	glUniformMatrix4fv(final_quad_shader.uniform[rp_final][su_inv_view],
		1, GL_TRUE, (float *)cam.inv_view);
	glUniformMatrix4fv(final_quad_shader.uniform[rp_final][su_inv_proj],
		1, GL_TRUE, (float *)cam.inv_proj);

	glBindVertexArray(final_quad_shader.vao[rp_final][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fqfx->fb_tex);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fqfx->fb_depth);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
