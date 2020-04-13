#ifndef LINALG_H
#define LINALG_H
struct mat4 {
	float v[4][4];
};

struct vec4 {
	float x;
	float y;
	float z;
	float w;
};

struct camera {
	struct vec4 pos;
	struct vec4 tgt;
	struct vec4 up;
	float yaw;
	float pitch;
};

struct mat4 gen_look_at(struct camera c);
struct mat4 gen_look_at(struct camera);
struct mat4 gen_perspective_mat4(float r, float t, float n, float f);
//struct mat4 gen_rot_mat4(float t, struct vec4 r);
struct mat4 gen_scale_mat4(struct vec4 t);
struct mat4 gen_trans_mat4(struct vec4 t);
struct mat4 mat4_mult_mat4(struct mat4 a, struct mat4 b);
struct vec4 vec4_add(struct vec4 a, struct vec4 b);
struct vec4 vec4_cross(struct vec4 a, struct vec4 b);
struct vec4 vec4_normalize(struct vec4 v);
struct vec4 vec4_scale(struct vec4 v, float s);
struct vec4 vec4_sub(struct vec4 a, struct vec4 b);
void print_matrix(struct mat4 m);
#endif
