#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "shared/math/linalg.h"

void
gen_trans_mat4(struct vec4 *t, struct mat4 *m)
{
	struct mat4 mr = {
		.v = {
			{ 1, 0, 0, t->x },
			{ 0, 1, 0, t->y },
			{ 0, 0, 1, t->z },
			{ 0, 0, 0, 1    },
		}
	};

	*m = mr;
}

void
gen_scale_mat4(struct vec4 *t, struct mat4 *m)
{
	struct mat4 mr = {
		.v = {
			{ t->x, 0,    0,    0 },
			{ 0,    t->y, 0,    0 },
			{ 0,    0,    t->z, 0 },
			{ 0,    0,    0,    1 },
		}
	};

	*m = mr;
}

void
gen_perspective_mat4(float fov, float aspect, float n, float f, struct mat4 *m)
{
	float t = tan(fov / 2) * n;
	float r = t * aspect;

	struct mat4 perspective = {
		.v = {
			n / r, 0, 0, 0,
			0, n / t, 0, 0,
			0, 0, -1 * ((f + n) / (f - n)), -2 * f * n / (f - n),
			0, 0, -1, 0
		}
	};

	*m = perspective;
}

void
mat4_mult_mat4(struct mat4 *a, struct mat4 *b, struct mat4 *m)
{
	m->v[0][0] = a->v[0][0] * b->v[0][0] + a->v[0][1] * b->v[1][0] + a->v[0][2] * b->v[2][0] + a->v[0][3] * b->v[3][0];
	m->v[0][1] = a->v[0][0] * b->v[0][1] + a->v[0][1] * b->v[1][1] + a->v[0][2] * b->v[2][1] + a->v[0][3] * b->v[3][1];
	m->v[0][2] = a->v[0][0] * b->v[0][2] + a->v[0][1] * b->v[1][2] + a->v[0][2] * b->v[2][2] + a->v[0][3] * b->v[3][2];
	m->v[0][3] = a->v[0][0] * b->v[0][3] + a->v[0][1] * b->v[1][3] + a->v[0][2] * b->v[2][3] + a->v[0][3] * b->v[3][3];

	m->v[1][0] = a->v[1][0] * b->v[0][0] + a->v[1][1] * b->v[1][0] + a->v[1][2] * b->v[2][0] + a->v[1][3] * b->v[3][0];
	m->v[1][1] = a->v[1][0] * b->v[0][1] + a->v[1][1] * b->v[1][1] + a->v[1][2] * b->v[2][1] + a->v[1][3] * b->v[3][1];
	m->v[1][2] = a->v[1][0] * b->v[0][2] + a->v[1][1] * b->v[1][2] + a->v[1][2] * b->v[2][2] + a->v[1][3] * b->v[3][2];
	m->v[1][3] = a->v[1][0] * b->v[0][3] + a->v[1][1] * b->v[1][3] + a->v[1][2] * b->v[2][3] + a->v[1][3] * b->v[3][3];

	m->v[2][0] = a->v[2][0] * b->v[0][0] + a->v[2][1] * b->v[1][0] + a->v[2][2] * b->v[2][0] + a->v[2][3] * b->v[3][0];
	m->v[2][1] = a->v[2][0] * b->v[0][1] + a->v[2][1] * b->v[1][1] + a->v[2][2] * b->v[2][1] + a->v[2][3] * b->v[3][1];
	m->v[2][2] = a->v[2][0] * b->v[0][2] + a->v[2][1] * b->v[1][2] + a->v[2][2] * b->v[2][2] + a->v[2][3] * b->v[3][2];
	m->v[2][3] = a->v[2][0] * b->v[0][3] + a->v[2][1] * b->v[1][3] + a->v[2][2] * b->v[2][3] + a->v[2][3] * b->v[3][3];

	m->v[3][0] = a->v[3][0] * b->v[0][0] + a->v[3][1] * b->v[1][0] + a->v[3][2] * b->v[2][0] + a->v[3][3] * b->v[3][0];
	m->v[3][1] = a->v[3][0] * b->v[0][1] + a->v[3][1] * b->v[1][1] + a->v[3][2] * b->v[2][1] + a->v[3][3] * b->v[3][1];
	m->v[3][2] = a->v[3][0] * b->v[0][2] + a->v[3][1] * b->v[1][2] + a->v[3][2] * b->v[2][2] + a->v[3][3] * b->v[3][2];
	m->v[3][3] = a->v[3][0] * b->v[0][3] + a->v[3][1] * b->v[1][3] + a->v[3][2] * b->v[2][3] + a->v[3][3] * b->v[3][3];
}

void
gen_look_at(const struct camera *c, struct mat4 *m)
{
	struct vec4 right = { 0, 1, 0 }, up, dir = c->tgt;
	struct mat4 trans;

	vec4_normalize(&dir);

	vec4_cross(&right, &dir);
	vec4_normalize(&right);

	up = dir;
	vec4_cross(&up, &right);

	struct mat4 la = {
		.v = {
			right.x, right.y, right.z, 0,
			up.x,    up.y,    up.z,    0,
			dir.x,   dir.y,   dir.z,   0,
			0,       0,       0,       1
		}
	};

	dir = c->pos;
	vec4_scale(&dir, -1);

	gen_trans_mat4(&dir, &trans);
	mat4_mult_mat4(&la, &trans, m);
}

void
print_matrix(struct mat4 *m)
{
	printf(
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n",
		m->v[0][0], m->v[0][1], m->v[0][2], m->v[0][3],
		m->v[1][0], m->v[1][1], m->v[1][2], m->v[1][3],
		m->v[2][0], m->v[2][1], m->v[2][2], m->v[2][3],
		m->v[3][0], m->v[3][1], m->v[3][2], m->v[3][3]
		);

	printf("---\n");
}

void
vec4_cross(struct vec4 *a, struct vec4 *b)
{
	struct vec4 ret = {
		a->y * b->z - a->z * b->y,
		a->z * b->x - a->x * b->z,
		a->x * b->y - a->y * b->x,
		0
	};

	*a = ret;
}

void
vec4_normalize(struct vec4 *v)
{
	float mag = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

	if (mag == 0) {
		mag = 1;
	}

	v->x /= mag;
	v->y /= mag;
	v->z /= mag;
}

void
vec4_add(struct vec4 *a, struct vec4 *b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
}

void
vec4_sub(struct vec4 *a, struct vec4 *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}

void
vec4_scale(struct vec4 *v, float s)
{
	v->x *= s;
	v->y *= s;
	v->z *= s;
}
