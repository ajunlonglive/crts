#ifndef LINALG_H
#define LINALG_H
struct mat4 {
	float v[4][4];
};

struct vec4 {
	float x, y, z, w;
};

struct camera {
	struct vec4 pos;
	struct vec4 tgt;
	struct vec4 up;
	float yaw;
	float pitch;
};

void gen_trans_mat4(struct vec4 *t, struct mat4 *m);
void gen_scale_mat4(struct vec4 *t, struct mat4 *m);
void mat4_mult_mat4(struct mat4 *a, struct mat4 *b, struct mat4 *m);
void gen_perspective_mat4(float fov, float aspect, float n, float f, struct mat4 *m);
void gen_look_at(const struct camera *c, struct mat4 *m);

void print_matrix(struct mat4 *m);

void vec4_cross(struct vec4 *a, struct vec4 *b);
void vec4_normalize(struct vec4 *v);
void vec4_add(struct vec4 *a, struct vec4 *b);
void vec4_sub(struct vec4 *a, struct vec4 *b);
void vec4_scale(struct vec4 *v, float s);
#endif
