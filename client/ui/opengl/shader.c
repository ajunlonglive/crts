#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

static const struct {
	uint32_t id;
	const char *name;
	bool positional;
} default_uniform[render_pass_count][COUNT] = {
	[rp_final] = {
		{ duf_viewproj, "viewproj", true },
		{ duf_view_pos, "view_pos" },
		{ duf_light_space, "light_space" },
		{ duf_light_pos, "light_pos" },
	},
	[rp_depth] = {
		{ dud_light_space, "light_space" },
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
determine_attribute_storage(const struct shader_spec *spec, size_t (*size)[COUNT],
	enum render_pass rp)
{
	uint32_t i, j, max_buf = 0;

	for (j = 0; j < COUNT; ++j) {
		if (!spec->attribute[j][0].count) {
			break;
		}

		for (i = 0; i < COUNT; ++i) {
			if (!spec->attribute[j][i].count) {
				break;
			} else if (!spec->interleaved && rp == rp_depth
				   && !spec->attribute[j][i].positional) {
				continue;
			}

			if (spec->attribute[j][i].buffer > max_buf) {
				max_buf = spec->attribute[j][i].buffer;
			}

			size[j][spec->attribute[j][i].buffer] +=
				spec->attribute[j][i].count
				* gl_type_to_size(spec->attribute[j][i].type);
		}
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

static bool
locate_uniforms(const struct shader_spec *spec, struct shader *shader, enum render_pass rp)
{
	uint32_t i;
	int32_t uni;
	bool missing = false;

	/* default uniforms */
	for (i = 0; i < default_uniform_len[rp]; ++i) {
		if (spec->skip_lighting && !default_uniform[rp][i].positional) {
			shader->uniform[rp][default_uniform[rp][i].id] = -1;
			continue;
		}

		uni = glGetUniformLocation(shader->id[rp],
			default_uniform[rp][i].name);

		if (uni < 0) {
			LOG_W("uniform: %s not found", default_uniform[rp][i].name);
			missing = true;
		}

		shader->uniform[rp][default_uniform[rp][i].id] = uni;
	}

	/* user uniforms */
	for (i = 0; i < COUNT; ++i) {
		if (!spec->uniform[rp][i].name) {
			break;
		}

		uni = glGetUniformLocation(shader->id[rp],
			spec->uniform[rp][i].name);

		if (uni < 0) {
			LOG_W("uniform: %s not found", default_uniform[rp][i].name);
			missing = true;
		}

		shader->uniform[rp][spec->uniform[rp][i].id] = uni;
	}

	return !missing;
}

static void
setup_vertex_attrib_arrays(const struct shader_spec *spec, struct shader *shader,
	enum render_pass rp)
{
	uint32_t i, j, buf, attrib = 0;
	glBindVertexArray(0);

	size_t size[COUNT][COUNT] = { 0 };
	determine_attribute_storage(spec, size, rp);

	for (j = 0; j < COUNT; ++j) {
		if (!spec->attribute[j][0].count) {
			break;
		}

		glGenVertexArrays(1, &shader->vao[rp][j]);

		/* bind the vao */
		glBindVertexArray(shader->vao[rp][j]);

		/* bind the ebo */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shader->buffer[bt_ebo]);

		size_t off[COUNT] = { 0 };

		attrib = 0;

		for (i = 0; i < COUNT; ++i) {
			if (!spec->attribute[j][i].count) {
				break;
			} else if (rp == rp_depth && !spec->attribute[j][i].positional) {
				continue;
			}

			buf = spec->attribute[j][i].buffer;

			glBindBuffer(GL_ARRAY_BUFFER, shader->buffer[buf]);

			glVertexAttribPointer(attrib, spec->attribute[j][i].count,
				spec->attribute[j][i].type, GL_FALSE,
				size[j][buf],
				(void *)(spec->attribute[j][i].offset + off[buf]));

			/* L("  %d -> buf: %d, count: %d, stride: %d, off: %d, div: %d", */
			/* 	attrib, */
			/* 	shader->buffer[buf], */
			/* 	spec->attribute[j][i].count, */
			/* 	size[j][buf], */
			/* 	spec->attribute[j][i].offset + off[buf], */
			/* 	spec->attribute[j][i].divisor */
			/* 	); */

			glEnableVertexAttribArray(attrib);

			if (spec->attribute[j][i].divisor) {
				glVertexAttribDivisor(attrib, spec->attribute[j][i].divisor);
			}

			off[buf] += spec->attribute[j][i].count
				    * gl_type_to_size(spec->attribute[j][i].type);

			++attrib;
		}

		/* unbind */
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/* unbind */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool
shader_create(const struct shader_spec *spec, struct shader *shader)
{
	uint32_t i;
	enum render_pass rp;
	size_t size[COUNT][COUNT] = { 0 };
	size_t bufs = determine_attribute_storage(spec, size, rp_final);

	/* generate the buffers we know we need */
	glGenBuffers(bufs + 1, shader->buffer);

	for (rp = 0; rp < render_pass_count; ++rp) {
		/* L("rp: %d", rp); */
		if (!spec->src[rp][0].path) {
			continue;
		}

		/* link shaders */
		if (!link_shaders((struct shader_src *)spec->src[rp], &shader->id[rp])) {
			LOG_W("missing uniforms in one of");
			for (i = 0; i < 3 && spec->src[rp][i].path; ++i) {
				LOG_W("--> %s", spec->src[rp][i].path);
			}

			return false;
		}

		/* locate uniforms */
		if (!locate_uniforms(spec, shader, rp)) {
			LOG_W("missing uniforms in one of");
			for (i = 0; i < 3 && spec->src[rp][i].path; ++i) {
				LOG_W("--> %s", spec->src[rp][i].path);
			}
		}

		/* setup attributes */
		setup_vertex_attrib_arrays(spec, shader, rp);
	}

	/* send initial data */
	shader_upload_data(shader, spec->static_data);

	return true;
}

/* we set all uniforms even though not all may be there, but according to the
 * docs using a location of -1 makes the call do nothing anyway, there isn't
 * really any benefit to wrapping it in an if statement */
void
shader_check_def_uni(const struct shader *shader, struct opengl_ui_ctx *ctx)
{
	switch (ctx->pass) {
	case rp_final:
		if (cam.changed) {
			glUniformMatrix4fv(
				shader->uniform[rp_final][duf_viewproj],
				1, GL_TRUE, (float *)cam.proj);

			glUniform3fv(shader->uniform[rp_final][duf_view_pos],
				1, cam.pos);
		}

		if (sun.changed) {
			glUniform3fv(shader->uniform[rp_final][duf_light_pos],
				1, sun.pos);

			glUniformMatrix4fv(
				shader->uniform[rp_final][duf_light_space],
				1, GL_TRUE, (float *)sun.proj);
		}
		break;
	case rp_depth:
		if (sun.changed) {
			glUniformMatrix4fv(
				shader->uniform[rp_depth][duf_light_space],
				1, GL_TRUE, (float *)sun.proj);
		}
		break;
	default:
		break;
	}
}
