#ifndef INCLUDE_CLIENT_UI_OPENGL_SHADER_H
#define INCLUDE_CLIENT_UI_OPENGL_SHADER_H

#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/loaders/shader.h"

#define COUNT 32

struct shader_spec {
	struct shader_src src[16];
	struct {
		uint32_t id;
		const char *name;
	} uniform[COUNT];

	struct {
		uint32_t count;
		GLenum type;
		bool instanced;
	} attribute[COUNT];

	struct {
		const uint32_t *indices;
		size_t indices_len;
		const float *verts;
		size_t verts_len;
	} object;

	enum render_pass pass;
};

enum default_uniform_rp_final {
	du_proj,
	du_view,
	du_view_pos,
	default_uniform_rp_final_count
};

enum default_uniform_rp_depth {
	du_light_space,
	default_uniform_rp_depth_count
};

#define UNIFORM_START_RP_FINAL default_uniform_rp_final_count
#define UNIFORM_START_RP_DEPTH default_uniform_rp_depth_count

enum buffer_type {
	bt_vbo,
	bt_ebo,
	bt_ivbo,
};

struct shader {
	uint32_t id, vao, uniform[COUNT], buffer[3];
	enum render_pass pass;
};

bool shader_create(const struct shader_spec *spec, struct shader *shader);
void shader_use(const struct shader *shader);
void shader_check_def_uni(const struct shader *shader, struct opengl_ui_ctx *ctx);
#endif
