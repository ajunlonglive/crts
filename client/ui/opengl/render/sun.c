#include "posix.h"

#include <assert.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render/sun.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

struct shader sun_shader;

bool
render_world_setup_sun(void)
{
	struct shader_spec spec = {
		.src = {
			[rp_final] =  {
				{ "sun.vert", GL_VERTEX_SHADER },
				{ "sun.frag", GL_FRAGMENT_SHADER },
			}
		},
		.attribute = { { { 3, GL_FLOAT, bt_vbo } } },
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj),
		}
	};

	return shader_create(&spec, &sun_shader);
}

void
render_sun_setup_frame(struct opengl_ui_ctx *ctx)
{
	glBindBuffer(GL_ARRAY_BUFFER, sun_shader.buffer[bt_vbo]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, sun.pos, GL_DYNAMIC_DRAW);
}

void
render_sun(struct opengl_ui_ctx *ctx)
{
	glUseProgram(sun_shader.id[rp_final]);
	shader_check_def_uni(&sun_shader, ctx);

	glBindVertexArray(sun_shader.vao[rp_final][0]);

	glPointSize(50);
	glDrawArrays(GL_POINTS, 0, 1);
}
