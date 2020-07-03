#ifndef LINALG_H
#define LINALG_H

#include <stdbool.h>

typedef float mat4[4][4];
typedef float vec4[4];
typedef float vec3[3];

struct camera {
	vec4 pos;
	vec4 tgt;
	vec4 up;
	float yaw;
	float pitch;
	bool changed, unlocked;
};

void cam_calc_tgt(struct camera *cam);

void gen_trans_mat4(vec4 t, mat4 m);
void gen_scale_mat4(vec4 t, mat4 m);
void mat4_mult_mat4(mat4 a, mat4 b, mat4 m);
void gen_perspective_mat4(float fov, float aspect, float n, float f, mat4 m);
void gen_ortho_mat4(float fov, float aspect, float n, float f, mat4 m);
void gen_look_at(const struct camera *c, mat4 m);

void print_matrix(mat4 m);

void vec4_cross(vec4 a, vec4 b);
void vec4_normalize(vec4 v);
void vec4_add(vec4 a, vec4 b);
void vec4_sub(vec4 a, vec4 b);
void vec4_scale(vec4 v, float s);

void calc_normal(vec4 a, vec4 b, vec4 c, vec4 norm);
#endif
