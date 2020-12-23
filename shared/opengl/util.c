#include "posix.h"

#include <assert.h>
#include <stdbool.h>

#include "shared/opengl/util.h"
#include "shared/util/file_formats/load_tga.h"
#include "shared/util/log.h"

uint32_t
fb_attach_color(uint32_t w, uint32_t h)
{
	uint32_t tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tex, 0);

	return tex;
}

uint32_t
fb_attach_db(uint32_t w, uint32_t h)
{
	uint32_t db;
	glGenRenderbuffers(1, &db);

	glBindRenderbuffer(GL_RENDERBUFFER, db);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, db);

	return db;
}

uint32_t
fb_attach_dtex(uint32_t w, uint32_t h)
{
	uint32_t dtex;

	glGenTextures(1, &dtex);
	glBindTexture(GL_TEXTURE_2D, dtex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, dtex, 0);

	return dtex;
}

int32_t
load_tex(char *asset, GLenum wrap, GLenum filter)
{
	uint32_t tex;
	uint16_t width, height;
	uint8_t bits;
	const void *data;
	GLenum fmt;

	if (!(data = load_tga(asset, &width, &height, &bits))) {
		return -1;
	}

	if (bits == 32) {
		fmt = GL_RGBA;
	} else if (bits == 24) {
		fmt = GL_RGB;
	} else {
		assert(false);
		return -1;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt,
		GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return tex;
}
