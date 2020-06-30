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
		uint8_t buffer;
		uint8_t divisor;
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
	bt_ebo,
	bt_vbo,
	bt_ivbo,
	bt_buf4, bt_buf5, bt_buf6, bt_buf7, bt_buf8, bt_buf9, bt_buf10, bt_buf11,
	bt_buf12, bt_buf13, bt_buf14, bt_buf15, bt_buf16, bt_buf17, bt_buf18,
	bt_buf19, bt_buf20, bt_buf21, bt_buf22, bt_buf23, bt_buf24, bt_buf25,
	bt_buf26, bt_buf27, bt_buf28, bt_buf29, bt_buf30, bt_buf31, bt_buf32,
};


struct shader {
	uint32_t id, vao, uniform[COUNT], buffer[COUNT];
	enum render_pass pass;
};

bool shader_create(const struct shader_spec *spec, struct shader *shader);
void shader_use(const struct shader *shader);
void shader_check_def_uni(const struct shader *shader, struct opengl_ui_ctx *ctx);
#endif
