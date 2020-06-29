#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

static struct {
	uint32_t id;
	const char *name;
} default_uniform[default_uniform_count] = {
	{ du_view, "view" },
	{ du_proj, "proj" },
	{ du_view_pos, "view_pos" },
};

static size_t
gl_type_to_size(GLenum type)
{
	switch (type) {
	case GL_FLOAT:
		return sizeof(float);
	default:
		return 0;
	}
}

static size_t *
determine_attribute_size(const struct shader_spec *spec)
{
	static size_t size[2] = { 0 };
	uint32_t i;

	size[0] = size[1] = 0;

	for (i = 0; i < COUNT; ++i) {
		if (!spec->attribute[i].count) {
			break;
		}

		size[spec->attribute[i].instanced] +=
			spec->attribute[i].count * gl_type_to_size(spec->attribute[i].type);
	}

	return size;
}

bool
shader_create(const struct shader_spec *spec, struct shader *shader)
{
	uint32_t i;
	bool instanced;
	size_t *size = determine_attribute_size(spec), off[2] = { 0 };

	/* link shaders */
	if (!link_shaders((struct shader_src *)spec->src, &shader->id)) {
		return false;
	}

	/* setup uniforms */
	for (i = 0; i < default_uniform_count; ++i) {
		shader->uniform[default_uniform[i].id] =
			glGetUniformLocation(shader->id, default_uniform[i].name);
	}

	for (i = 0; i < COUNT; ++i) {
		if (!spec->uniform[i].name) {
			break;
		}

		shader->uniform[spec->uniform[i].id] =
			glGetUniformLocation(shader->id, spec->uniform[i].name);
	}

	glUseProgram(shader->id);

	/* setup attributes */

	/* generate the buffers we know we need */
	glGenBuffers(size[1] ? 3 : 2, shader->buffer);
	glGenVertexArrays(1, &shader->vao);

	/* bind the vao */
	glBindVertexArray(shader->vao);

	for (i = 0; i < COUNT; ++i) {
		if (!spec->attribute[i].count) {
			break;
		}

		instanced = spec->attribute[i].instanced;

		glBindBuffer(GL_ARRAY_BUFFER,
			shader->buffer[instanced ? bt_ivbo : bt_vbo]);

		glVertexAttribPointer(i, spec->attribute[i].count,
			spec->attribute[i].type, GL_FALSE,
			size[instanced],
			(void *)off[instanced]);

		glEnableVertexAttribArray(i);

		off[instanced] +=
			spec->attribute[i].count * gl_type_to_size(spec->attribute[i].type);
	}

	/* send initial data */
	if (spec->object.indices) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader->buffer[bt_ebo]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, spec->object.indices_len,
			spec->object.indices, GL_STATIC_DRAW);
	}

	if (spec->object.verts) {
		glBindBuffer(GL_ARRAY_BUFFER, shader->buffer[bt_vbo]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, spec->object.verts_len,
			spec->object.verts, GL_STATIC_DRAW);
	}

	/* unbind */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void
shader_use(const struct shader *shader)
{
	glUseProgram(shader->id);
	glBindVertexArray(shader->vao);
}

void
shader_check_cam(const struct shader *shader, struct opengl_ui_ctx *ctx)
{
	if (cam.changed) {
		glUniformMatrix4fv(shader->uniform[du_proj], 1, GL_TRUE, (float *)ctx->mproj);
		glUniformMatrix4fv(shader->uniform[du_view], 1, GL_TRUE, (float *)ctx->mview);
		glUniform3fv(shader->uniform[du_view_pos], 1, cam.pos);
	}
}
