#ifndef INCLUDE_CLIENT_UI_OPENGL_SHADER_MULTI_OBJ_H
#define INCLUDE_CLIENT_UI_OPENGL_SHADER_MULTI_OBJ_H

#include "client/ui/opengl/shader.h"

struct model_spec {
	char *asset;
	float scale;
};

struct shader_multi_obj {
	struct shader shader;
	struct {
		struct darr *position, *lighting;
		size_t indices, index_offset;
		uint32_t buf[2];
	} obj_data[COUNT];
	size_t len;
};

typedef float obj_data[7];

bool shader_create_multi_obj(struct model_spec *ms, size_t mslen,
	struct shader_multi_obj *smo);
void smo_clear(struct shader_multi_obj *smo);
void smo_push(struct shader_multi_obj *smo, uint32_t i, obj_data data);
void smo_upload(struct shader_multi_obj *smo);
void smo_draw(struct shader_multi_obj *smo, struct opengl_ui_ctx *ctx);
#endif
