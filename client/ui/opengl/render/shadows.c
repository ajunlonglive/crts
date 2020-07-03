#include "posix.h"

#include <assert.h>

#include "client/ui/opengl/render/shadows.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/log.h"

bool
render_world_setup_shadows(struct shadow_map *sm)
{
	glGenFramebuffers(1, &sm->depth_map_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, sm->depth_map_fb);

	glGenTextures(1, &sm->depth_map_tex);
	glBindTexture(GL_TEXTURE_2D, sm->depth_map_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sm->dim, sm->dim, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, sm->depth_map_tex, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}
