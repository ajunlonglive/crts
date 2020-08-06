#include "posix.h"

#include "shared/opengl/loaders/shader.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"

#define BUFLEN 0xfff

static bool
compile_shader(const char *path, GLenum type, uint32_t *id)
{
	int32_t ret;

	struct file_data *fdat;

	if (!(fdat = asset(path))) {
		return false;
	}

	const GLchar *src = (GLchar *)fdat->data;

	*id = glCreateShader(type);
	glShaderSource(*id, 1, &src, NULL);
	glCompileShader(*id);

	char buf[BUFLEN];
	glGetShaderiv(*id, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		glGetShaderInfoLog(*id, BUFLEN, NULL, (char *)buf);
		LOG_W("failed to compile '%s'\n%s", path, buf);
		return false;
	}

	return true;
}

bool
link_shaders(struct shader_src *shaders, uint32_t *program)
{
	size_t i = 0;
	int32_t ret;
	char errinfo[BUFLEN];

	*program = glCreateProgram();

	for (i = 0; shaders[i].path && *shaders[i].path; ++i) {
		if (!compile_shader(shaders[i].path, shaders[i].type,
			&shaders[i].id)) {
			return false;
		}
		glAttachShader(*program, shaders[i].id);
	}

	glLinkProgram(*program);

	glGetProgramiv(*program, GL_LINK_STATUS, &ret);
	if (!ret) {
		glGetProgramInfoLog(*program, BUFLEN, NULL, errinfo);
		LOG_W("failed to link program\n%s", errinfo);
		LOG_W("program:");
		for (i = 0; shaders[i].path && *shaders[i].path; ++i) {
			LOG_W("--> %s", shaders[i].path);
		}
		return false;
	}

	for (i = 0; shaders[i].path && *shaders[i].path; ++i) {
		glDeleteShader(shaders[i].id);
	}

	return true;
}
