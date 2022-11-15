#include "posix.h"

#include "client/ui/gl/globals.h"
#include "client/ui/gl/shader.h"
#include "client/ui/gl/ui.h"
#include "shared/ui/gl/shader.h"

/* we set all uniforms even though not all may be there, but according to the
 * docs using a location of -1 makes the call do nothing anyway, there isn't
 * really any benefit to wrapping it in an if statement */
void
shader_check_def_uni(const struct shader *shader, struct gl_ui_ctx *ctx)
{
	switch (ctx->pass) {
	case rp_final:
	{
		float clip_plane[] = {
			ctx->clip_plane == 0,
			ctx->clip_plane == 1,
			ctx->clip_plane == 2,
		};

		glUniform3fv(shader->uniform[rp_final][duf_clip_plane], 1, clip_plane);
	}

		glUniformMatrix4fv(
			shader->uniform[rp_final][duf_viewproj],
			1, GL_TRUE, (float *)cam.viewproj);

		glUniform3fv(shader->uniform[rp_final][duf_view_pos],
			1, cam.pos);

		glUniform3fv(shader->uniform[rp_final][duf_light_pos],
			1, sun.pos);

		glUniformMatrix4fv(
			shader->uniform[rp_final][duf_light_space],
			1, GL_TRUE, (float *)sun.viewproj);
		break;
	case rp_depth:
		glUniformMatrix4fv(
			shader->uniform[rp_depth][duf_light_space],
			1, GL_TRUE, (float *)sun.viewproj);
		break;
	default:
		break;
	}
}
