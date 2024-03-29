#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shared/math/geom.h"
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
gen_ortho_mat4_from_lrbt(float l, float r, float b, float t, mat4 m)
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
mat4_mult_vec4(mat4 a, const vec4 b, vec4 r)
{
	r[0] = a[0][0] * b[0] + a[0][1] * b[1] + a[0][2] * b[2] + a[0][3] * b[3];
	r[1] = a[1][0] * b[0] + a[1][1] * b[1] + a[1][2] * b[2] + a[1][3] * b[3];
	r[2] = a[2][0] * b[0] + a[2][1] * b[1] + a[2][2] * b[2] + a[2][3] * b[3];
	r[3] = a[3][0] * b[0] + a[3][1] * b[1] + a[3][2] * b[2] + a[3][3] * b[3];
}

void
mat4_transpose(mat4 a)
{
	float tmp;
#define SWAP(ai, aj, bi, bj) tmp = a[ai][aj]; a[ai][aj] = a[bi][bj]; a[bi][bj] = tmp
	/* 0, 0, 0, 0 */ SWAP(0, 1, 1, 0); SWAP(0, 2, 2, 0); SWAP(0, 3, 3, 0);
	/*               0, 0, 0, 0     */ SWAP(1, 2, 2, 1); SWAP(1, 3, 3, 1);
	/*                                 0, 0, 0, 0     */ SWAP(2, 3, 3, 2);
	/*                                                   0, 0, 0, 0     */
#undef SWAP
}

bool
mat4_invert(mat4 m_in, mat4 m_out)
{
	/* from mesa */
	const float *m = (float *)m_in;
	float *invOut = (float *)m_out;
	float inv[16], det;
	int i;

	inv[0] = m[5]  * m[10] * m[15] -
		 m[5]  * m[11] * m[14] -
		 m[9]  * m[6]  * m[15] +
		 m[9]  * m[7]  * m[14] +
		 m[13] * m[6]  * m[11] -
		 m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
		 m[4]  * m[11] * m[14] +
		 m[8]  * m[6]  * m[15] -
		 m[8]  * m[7]  * m[14] -
		 m[12] * m[6]  * m[11] +
		 m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] -
		 m[4]  * m[11] * m[13] -
		 m[8]  * m[5] * m[15] +
		 m[8]  * m[7] * m[13] +
		 m[12] * m[5] * m[11] -
		 m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] +
		  m[4]  * m[10] * m[13] +
		  m[8]  * m[5] * m[14] -
		  m[8]  * m[6] * m[13] -
		  m[12] * m[5] * m[10] +
		  m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
		 m[1]  * m[11] * m[14] +
		 m[9]  * m[2] * m[15] -
		 m[9]  * m[3] * m[14] -
		 m[13] * m[2] * m[11] +
		 m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
		 m[0]  * m[11] * m[14] -
		 m[8]  * m[2] * m[15] +
		 m[8]  * m[3] * m[14] +
		 m[12] * m[2] * m[11] -
		 m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] +
		 m[0]  * m[11] * m[13] +
		 m[8]  * m[1] * m[15] -
		 m[8]  * m[3] * m[13] -
		 m[12] * m[1] * m[11] +
		 m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] -
		  m[0]  * m[10] * m[13] -
		  m[8]  * m[1] * m[14] +
		  m[8]  * m[2] * m[13] +
		  m[12] * m[1] * m[10] -
		  m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] -
		 m[1]  * m[7] * m[14] -
		 m[5]  * m[2] * m[15] +
		 m[5]  * m[3] * m[14] +
		 m[13] * m[2] * m[7] -
		 m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] +
		 m[0]  * m[7] * m[14] +
		 m[4]  * m[2] * m[15] -
		 m[4]  * m[3] * m[14] -
		 m[12] * m[2] * m[7] +
		 m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] -
		  m[0]  * m[7] * m[13] -
		  m[4]  * m[1] * m[15] +
		  m[4]  * m[3] * m[13] +
		  m[12] * m[1] * m[7] -
		  m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] +
		  m[0]  * m[6] * m[13] +
		  m[4]  * m[1] * m[14] -
		  m[4]  * m[2] * m[13] -
		  m[12] * m[1] * m[6] +
		  m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		 m[1] * m[7] * m[10] +
		 m[5] * m[2] * m[11] -
		 m[5] * m[3] * m[10] -
		 m[9] * m[2] * m[7] +
		 m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		 m[0] * m[7] * m[10] -
		 m[4] * m[2] * m[11] +
		 m[4] * m[3] * m[10] +
		 m[8] * m[2] * m[7] -
		 m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		  m[0] * m[7] * m[9] +
		  m[4] * m[1] * m[11] -
		  m[4] * m[3] * m[9] -
		  m[8] * m[1] * m[7] +
		  m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		  m[0] * m[6] * m[9] -
		  m[4] * m[1] * m[10] +
		  m[4] * m[2] * m[9] +
		  m[8] * m[1] * m[6] -
		  m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0) {
		return false;
	}

	det = 1.0 / det;

	for (i = 0; i < 16; i++) {
		invOut[i] = inv[i] * det;
	}

	return true;
}

void
gen_look_at(const struct camera *c, mat4 m)
{
	vec4 right = { 0, 1, 0 }, up,
	     dir = { c->tgt[0], c->tgt[1], c->tgt[2], c->tgt[3] };
	mat4 trans;

	vec_normalize(dir);

	vec_cross(right, dir);
	vec_normalize(right);

	memcpy(up, dir, sizeof(vec4));
	vec_cross(up, right);

	mat4 la = {
		right[0], right[1], right[2], 0,
		up[0],    up[1],    up[2],    0,
		dir[0],   dir[1],   dir[2],   0,
		0,        0,        0,        1
	};

	memcpy(dir, c->pos, sizeof(vec4));
	vec_scale(dir, -1);

	gen_trans_mat4(dir, trans);
	mat4_mult_mat4(la, trans, m);
}

void
cam_calc_tgt(struct camera *cam)
{
	cam->tgt[0] = cos(cam->yaw) * cos(cam->pitch);
	cam->tgt[1] = sin(cam->pitch);
	cam->tgt[2] = sin(cam->yaw) * cos(cam->pitch);

	float aspect = cam->width / cam->height;

	switch (cam->proj_type) {
	case proj_perspective:
		gen_perspective_mat4(cam->fov, aspect, cam->near, cam->far,
			cam->proj);
		break;
	case proj_orthographic:
		gen_ortho_mat4(cam->fov, aspect, cam->near, cam->far,
			cam->proj);
		break;
	default:
		assert(false);
		return;
	}

	gen_look_at(cam, cam->view);

	mat4_mult_mat4(cam->proj, cam->view, cam->viewproj);

	mat4_invert(cam->proj, cam->inv_proj);
	mat4_invert(cam->view, cam->inv_view);
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
vec_cross(vec3 a, const vec3 b)
{
	vec3 ret = {
		a[1] * b[2] - a[2] * b[1],
		a[2] * b[0] - a[0] * b[2],
		a[0] * b[1] - a[1] * b[0],
	};

	a[0] = ret[0];
	a[1] = ret[1];
	a[2] = ret[2];
}

void
vec_normalize(vec3 v)
{
	float mag = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (mag == 0) {
		return;
	}

	v[0] /= mag;
	v[1] /= mag;
	v[2] /= mag;
}

void
vec_add(vec3 a, const vec3 b)
{
	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
}

void
vec_sub(vec3 a, const vec3 b)
{
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
}

void
vec_scale(vec3 v, float s)
{
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
}

float
vec_dot(const vec3 v1, const vec3 v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
vec4_dot(const vec4 v1, const vec4 v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
}

float
vec_mag(const vec3 v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void
make_plane(const vec3 p1, const vec3 p2, const vec3 p3, vec4 plane)
{
	calc_normal(p1, p2, p3, plane);
	plane[3] = -(vec_dot(plane, p1));
}

void
calc_normal(const float *a, const float *b, const float *c, float *norm)
{
	vec4 v1;

	memcpy(norm, b, sizeof(float) * 3);
	memcpy(v1, c, sizeof(float) * 3);

	vec_sub(norm, a);
	vec_sub(v1, a);
	vec_cross(norm, v1);
}

float
sqdist3d(float *a, float *b)
{
	float v[3] = { (b[0] - a[0]), (b[1] - a[1]), (b[2] - a[2]) };

	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

/*
 * Havel and Herout's Yet Faster Ray-Triangle Intersection algorithm.
 */

typedef struct {
	vec3 n0; float d0;
	vec3 n1; float d1;
	vec3 n2; float d2;
} isect_hh_data;
typedef union {
	float f;
	int i;
} int_or_float;
#define ISECT_NEAR 0.01f
#define ISECT_FAR 1000.0f

static void
isect_hh_pre(const vec3 v0, const vec3 v1, const vec3 v2, isect_hh_data *D)
{
	vec3 e1 = { 0 };
	vec_add(e1, v1);
	vec_sub(e1, v0);

	vec3 e2 = { 0 };
	vec_add(e2, v2);
	vec_sub(e2, v0);

	vec_add(D->n0, e1);
	vec_cross(D->n0, e2);
	D->d0 = vec_dot(D->n0, v0);

	float inv_denom = 1.0f / vec_dot(D->n0, D->n0);

	vec_cross(e2, D->n0);
	vec_add(D->n1, e2);
	vec_scale(D->n1, inv_denom);
	D->d1 = -vec_dot(D->n1, v0);

	vec_add(D->n2, D->n0);
	vec_cross(D->n2, e1);
	vec_scale(D->n2, inv_denom);
	D->d2 = -vec_dot(D->n2, v0);
}

static bool
isect_hh(const vec3 o, const vec3 d, isect_hh_data *D)
{
	float det = vec_dot(D->n0, d);
	float dett = D->d0 - vec_dot(o, D->n0);

	vec3 wr = { 0 }, tmp = { 0 };
	vec_add(wr, o);
	vec_scale(wr, det);

	vec_add(tmp, d);
	vec_scale(tmp, dett);
	vec_add(wr, tmp);

	float _t, *t = &_t;
	struct pointf _uv, *uv = &_uv;

	uv->x = vec_dot(wr, D->n1) + det * D->d1;
	uv->y = vec_dot(wr, D->n2) + det * D->d2;
	int_or_float tmpdet0 = { .f = det - uv->x - uv->y };
	int pdet0 = tmpdet0.i;
	int pdetu = (int_or_float) { .f = uv->x }.i;
	int pdetv = (int_or_float) { .f = uv->y }.i;
	pdet0 = pdet0 ^ pdetu;
	pdet0 = pdet0 | (pdetu ^ pdetv);
	if (pdet0 & 0x80000000) {
		return false;
	}
	float rdet = 1 / det;
	uv->x *= rdet;
	uv->y *= rdet;
	*t = dett * rdet;
	return *t >= ISECT_NEAR && *t <= ISECT_FAR;
}

bool
ray_intersects_tri(const float *origin, const float *dir,
	const float *t0, const float *t1, const float *t2)
{
	isect_hh_data D = { 0 };
	isect_hh_pre(t0, t1, t2, &D);
	return isect_hh(origin, dir, &D);
}
