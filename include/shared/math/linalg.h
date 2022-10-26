#ifndef LINALG_H
#define LINALG_H

#include <stdbool.h>

typedef float mat4[4][4];
typedef float vec4[4];
typedef float vec3[3];

enum projection { proj_perspective, proj_orthographic, };

struct camera { vec4 pos; vec4 tgt; vec4 up; float yaw; float pitch; bool
			changed, unlocked;

		enum projection proj_type; float width, height; float near, far; float
			fov;

		mat4 proj; };

void cam_calc_tgt(struct camera *cam);

void gen_trans_mat4(vec4 t, mat4 m); void gen_scale_mat4(vec4 t, mat4 m); void mat4_mult_mat4(mat4 a, mat4 b, mat4 m); void gen_perspective_mat4(float fov,
	float aspect, float n, float f, mat4 m); void gen_ortho_mat4(float fov, float aspect, float n, float f, mat4 m); void gen_ortho_mat4_from_lrbt(float l, float r, float b, float t, mat4 m); void gen_look_at(const struct camera *c, mat4 m);

void print_matrix(mat4 m);

void vec_cross(vec3 a, vec3 b);
void vec_normalize(vec3 v);
void vec_add(vec3 a, vec3 b);
void vec_sub(vec3 a, vec3 b);
void vec_scale(vec3 v, float s);
float vec_dot(vec3 v1, vec3 v2);

void calc_normal(float *a, float *b, float *c, float *norm);

float sqdist3d(float *a, float *b);
#endif
