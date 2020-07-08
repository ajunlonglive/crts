#include "posix.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shared/math/linalg.h"

void
gen_trans_mat4(vec4 t, mat4 m)
{
	m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = t[0];
	m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = t[1];
	m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = t[2];
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
}

void
gen_scale_mat4(vec4 t, mat4 m)
{
	m[0][0] = t[0]; m[0][1] = 0;    m[0][2] = 0;    m[0][3] = 0;
	m[1][0] = 0;    m[1][1] = t[1]; m[1][2] = 0;    m[1][3] = 0;
	m[2][0] = 0;    m[2][1] = 0;    m[2][2] = t[2]; m[2][3] = 0;
	m[3][0] = 0;    m[3][1] = 0;    m[3][2] = 0;    m[3][3] = 1;
}

void
gen_perspective_mat4(float fov, float aspect, float n, float f, mat4 m)
{
	float t = tan(fov / 2) * n;
	float r = t * aspect;

	m[0][0] = n / r;
	m[0][1] = 0;
	m[0][2] = 0;
	m[0][3] = 0;

	m[1][0] = 0;
	m[1][1] = n / t;
	m[1][2] = 0;
	m[1][3] = 0;

	m[2][0] = 0;
	m[2][1] = 0;
	m[2][2] = -1 * ((f + n) / (f - n));
	m[2][3] = -2 * f * n / (f - n);

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = -1;
	m[3][3] = 1;
}

void
gen_ortho_mat4(float fov, float aspect, float n, float f, mat4 m)
{
	float t = tan(fov / 2) * n;
	float r = t * aspect;

	m[0][0] = 1 / r;
	m[0][1] = 0;
	m[0][2] = 0;
	m[0][3] = 0;

	m[1][0] = 0;
	m[1][1] = 1 / t;
	m[1][2] = 0;
	m[1][3] = 0;

	m[2][0] = 0;
	m[2][1] = 0;
	m[2][2] = -2 / (f - n);
	m[2][3] = -1 * (f + n) / (f - n);
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
}

void
gen_fake_ortho_mat4(float l, float r, float b, float t, mat4 m)
{
	m[0][0] = 2 / (r - l);
	m[0][1] = 0;
	m[0][2] = 0;
	m[0][3] = -1 * (r + l) / (r - l);

	m[1][0] = 0;
	m[1][1] = 2 / (t - b);
	m[1][2] = 0;
	m[1][3] = -1 * (t + b) / (t - b);

	m[2][0] = 0;
	m[2][1] = 0;
	m[2][2] = -1;
	m[2][3] = -1;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
}

void
mat4_mult_mat4(mat4 a, mat4 b, mat4 m)
{
	m[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
	m[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
	m[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
	m[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];

	m[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
	m[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
	m[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
	m[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];

	m[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
	m[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
	m[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
	m[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];

	m[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
	m[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
	m[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
	m[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];
}

void
gen_look_at(const struct camera *c, mat4 m)
{
	vec4 right = { 0, 1, 0 }, up,
	     dir = { c->tgt[0], c->tgt[1], c->tgt[2], c->tgt[3] };
	mat4 trans;

	vec4_normalize(dir);

	vec4_cross(right, dir);
	vec4_normalize(right);

	memcpy(up, dir, sizeof(vec4));
	vec4_cross(up, right);

	mat4 la = {
		right[0], right[1], right[2], 0,
		up[0],    up[1],    up[2],    0,
		dir[0],   dir[1],   dir[2],   0,
		0,        0,        0,        1
	};

	memcpy(dir, c->pos, sizeof(vec4));
	vec4_scale(dir, -1);

	gen_trans_mat4(dir, trans);
	mat4_mult_mat4(la, trans, m);
}

void
cam_calc_tgt(struct camera *cam)
{
	cam->tgt[0] = cos(cam->yaw) * cos(cam->pitch);
	cam->tgt[1] = sin(cam->pitch);
	cam->tgt[2] = sin(cam->yaw) * cos(cam->pitch);

	mat4 proj, view;
	float aspect = cam->width / cam->height;

	switch (cam->proj_type) {
	case proj_perspective:
		gen_perspective_mat4(cam->fov, aspect, cam->near, cam->far,
			proj);
		break;
	case proj_orthographic:
		gen_ortho_mat4(cam->fov, aspect, cam->near, cam->far,
			proj);
		break;
	}

	gen_look_at(cam, view);

	mat4_mult_mat4(proj, view, cam->proj);
}

void
print_matrix(mat4 m)
{
	printf(
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n",
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]
		);

	printf("---\n");
}

void
vec4_cross(vec4 a, vec4 b)
{
	vec4 ret = {
		a[1] * b[2] - a[2] * b[1],
		a[2] * b[0] - a[0] * b[2],
		a[0] * b[1] - a[1] * b[0],
	};

	a[0] = ret[0];
	a[1] = ret[1];
	a[2] = ret[2];
}

void
vec4_normalize(vec4 v)
{
	float mag = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (mag == 0) {
		mag = 1;
	}

	v[0] /= mag;
	v[1] /= mag;
	v[2] /= mag;
}

void
vec4_add(vec4 a, vec4 b)
{
	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
}

void
vec4_sub(vec4 a, vec4 b)
{
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
}

void
vec4_scale(vec4 v, float s)
{
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
}

void
calc_normal(vec4 a, vec4 b, vec4 c, vec4 norm)
{
	vec4 v1;

	memcpy(norm, b, sizeof(float) * 3);
	memcpy(v1, c, sizeof(float) * 3);

	vec4_sub(norm, a);
	vec4_sub(v1, a);
	vec4_cross(norm, v1);
}

float
sqdist3d(vec4 a, vec4 b)
{
	float v[3] = { (b[0] - a[0]), (b[1] - a[1]), (b[2] - a[2]) };

	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}
