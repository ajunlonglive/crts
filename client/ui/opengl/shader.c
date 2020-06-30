#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

static const struct {
	uint32_t id;
	const char *name;
} default_uniform[render_pass_count][COUNT] = {
	[rp_final] = {
		{ du_view, "view" },
		{ du_proj, "proj" },
		{ du_view_pos, "view_pos" },
	},
	[rp_depth] = {
		{ du_light_space, "light_space" },
	},
};

static const size_t default_uniform_len[render_pass_count] = {
	[rp_final] = default_uniform_rp_final_count,
	[rp_depth] = default_uniform_rp_depth_count,
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

static size_t
determine_attribute_storage(const struct shader_spec *spec, size_t *size)
{
	uint32_t i, max_buf = 0;

	size[0] = size[1] = 0;

	for (i = 0; i < COUNT; ++i) {
		if (!spec->attribute[i].count) {
			break;
		}

		if (spec->attribute[i].buffer > max_buf) {
			max_buf = spec->attribute[i].buffer;
		}

		size[spec->attribute[i].buffer] +=
			spec->attribute[i].count * gl_type_to_size(spec->attribute[i].type);
	}

	return max_buf + 1;
}

void
shader_upload_data(struct shader *shader,
	const struct static_shader_data *to_upload)
{
	uint32_t i;

	for (i = 0; i < COUNT; ++i) {
		if (!to_upload[i].data) {
			break;
		}

		GLenum btype =
			to_upload[i].buffer == bt_ebo ? GL_ELEMENT_ARRAY_BUFFER
						      : GL_ARRAY_BUFFER;

		glBindBuffer(btype, shader->buffer[to_upload[i].buffer]);
		glBufferData(btype, to_upload[i].size, to_upload[i].data, GL_STATIC_DRAW);
	}
}

bool
shader_create(const struct shader_spec *spec, struct shader *shader)
{
	uint32_t i, buf;
	size_t size[COUNT] = { 0 };
	size_t bufs = determine_attribute_storage(spec, size), off[COUNT] = { 0 };

	/* link shaders */
	if (!link_shaders((struct shader_src *)spec->src, &shader->id)) {
		return false;
	}

	/* setup uniforms */
	for (i = 0; i < default_uniform_len[spec->pass]; ++i) {
		shader->uniform[default_uniform[spec->pass][i].id] =
			glGetUniformLocation(shader->id,
				default_uniform[spec->pass][i].name);
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
	glGenBuffers(bufs + 1, shader->buffer);
	glGenVertexArrays(1, &shader->vao);

	/* bind the vao */
	glBindVertexArray(shader->vao);

	for (i = 0; i < COUNT; ++i) {
		if (!spec->attribute[i].count) {
			break;
		}

		buf = spec->attribute[i].buffer;
		/* L("attrib: %d, buf: %d, count: %d, stride: %ld, off: %ld, div: %d", */
		/* 	i, */
		/* 	buf, */
		/* 	spec->attribute[i].count, */
		/* 	size[buf], */
		/* 	off[buf], */
		/* 	spec->attribute[i].divisor */
		/* 	); */

		glBindBuffer(GL_ARRAY_BUFFER, shader->buffer[buf]);

		glVertexAttribPointer(i, spec->attribute[i].count,
			spec->attribute[i].type, GL_FALSE,
			size[buf],
			(void *)off[buf]);

		glEnableVertexAttribArray(i);

		if (spec->attribute[i].divisor) {
			glVertexAttribDivisor(i, spec->attribute[i].divisor);
		}

		off[buf] +=
			spec->attribute[i].count * gl_type_to_size(spec->attribute[i].type);
	}

	/* send initial data */
	shader_upload_data(shader, spec->static_data);

	/* unbind */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/* copy settings */
	shader->pass = spec->pass;

	return true;
}

void
shader_use(const struct shader *shader)
{
	glUseProgram(shader->id);
	glBindVertexArray(shader->vao);
}

void
shader_check_def_uni(const struct shader *shader, struct opengl_ui_ctx *ctx)
{
	switch (shader->pass) {
	case rp_final:
		if (cam.changed) {
			glUniformMatrix4fv(shader->uniform[du_proj], 1, GL_TRUE, (float *)ctx->mproj);
			glUniformMatrix4fv(shader->uniform[du_view], 1, GL_TRUE, (float *)ctx->mview);
			glUniform3fv(shader->uniform[du_view_pos], 1, cam.pos);
		}
		break;
	case rp_depth:
		break;
	default:
		break;
	}
}
