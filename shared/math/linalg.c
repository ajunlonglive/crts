#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "shared/math/linalg.h"

/*
   struct mat4
   gen_rot_mat4(float t, struct vec4 r)
   {
        struct vec4 q = {
                sin(t / 2) * r.x,
                sin(t / 2) * r.y,
                sin(t / 2) * r.z,
                cos(t / 2),
        };


        float mag = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

        if (mag == 0) {
                mag = 1;
        }

        q.x /= mag;
        q.y /= mag;
        q.z /= mag;
        q.w /= mag;

        struct vec4 qs = { q.x * q.x, q.y * q.y, q.z * q.z, q.w * q.w };

        struct mat4 rm = {
                .v = {
                        {
                                1 - 2 * (qs.z + qs.y),
                                2 * (q.y * q.x - q.z * q.w),
                                2 * (q.y * q.w + q.z * q.x),
                                0,
                        },
                        {
                                2 * (q.x * q.y + q.w * q.z),
                                1 - 2 * (qs.z + qs.x),
                                2 * (q.z * q.y - q.x * q.w),
                                0,
                        },
                        {
                                2 * (q.x * q.z - q.w * q.y),
                                2 * (q.y * q.z - q.w * q.x),
                                1 - 2 * (qs.y + qs.x),
                                0,
                        },
                        { 0, 0, 0, 1 },
                }
        };

        return rm;
   }

   struct mat4
   gen_rot_mat4(float t, struct vec4 r)
   {
        float x = r.x, y = r.y, z = r.z, xx, yy, zz;

        xx = x * x;
        yy = y * y;
        zz = z * z;

        struct mat4 ret = {
                .v = {
                        cos(t) + xx * ( 1 - cos(t)),
                        x * y * (1 - cos(t)) - z * sin(t),
                        x * z * (1 - cos(t)) - x * sin(t),
                        0,

                        y * x * (1 - cos(t)) + z * sin(t),
                        cos(t) + yy * (1 - cos(t)),
                        y * z * (1 - cos(t)) - x * sin(t),
                        0,

                        z * x * (1 - cos(t)) - y * sin(t),
                        z * y * (1 - cos(t)) + x * sin(t),
                        cos(t) + zz * (1 - cos(t)),
                        0,

                        0, 0, 0, 1
                }
        };

        return ret;
   }
 */

struct mat4
gen_trans_mat4(struct vec4 t)
{
	struct mat4 trans = {
		.v = {
			{ 1, 0, 0, t.x },
			{ 0, 1, 0, t.y },
			{ 0, 0, 1, t.z },
			{ 0, 0, 0, 1   },
		}
	};

	return trans;
}

struct mat4
gen_scale_mat4(struct vec4 t)
{
	struct mat4 trans = {
		.v = {
			{ t.x, 0,   0,   0 },
			{ 0,   t.y, 0,   0 },
			{ 0,   0,   t.z, 0 },
			{ 0,   0,   0,   1 },
		}
	};

	return trans;
}

/*          f
 * +-------*-------+
 * |\\     |     //|
 * | \\    |n   // |
 * |  +----*----+  |
 * |  ||        |  |
 * |  ||        |  |
 * |  |r=-l     |  |
 * |  ||___t=-b_|  |
 * |  +---------+  |
 * | //         \\ |
 * |//           \\|
 * +---------------+
 */
struct mat4
gen_perspective_mat4(float fov, float aspect, float n, float f)
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

	return perspective;
}

struct vec4
vec4_cross(struct vec4 a, struct vec4 b)
{
	struct vec4 ret = {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
		0
	};

	return ret;
}

struct mat4
mat4_mult_mat4(struct mat4 a, struct mat4 b)
{
	struct mat4 c = {
		.v = {
			a.v[0][0] * b.v[0][0] + a.v[0][1] * b.v[1][0] + a.v[0][2] * b.v[2][0] + a.v[0][3] * b.v[3][0],
			a.v[0][0] * b.v[0][1] + a.v[0][1] * b.v[1][1] + a.v[0][2] * b.v[2][1] + a.v[0][3] * b.v[3][1],
			a.v[0][0] * b.v[0][2] + a.v[0][1] * b.v[1][2] + a.v[0][2] * b.v[2][2] + a.v[0][3] * b.v[3][2],
			a.v[0][0] * b.v[0][3] + a.v[0][1] * b.v[1][3] + a.v[0][2] * b.v[2][3] + a.v[0][3] * b.v[3][3],

			a.v[1][0] * b.v[0][0] + a.v[1][1] * b.v[1][0] + a.v[1][2] * b.v[2][0] + a.v[1][3] * b.v[3][0],
			a.v[1][0] * b.v[0][1] + a.v[1][1] * b.v[1][1] + a.v[1][2] * b.v[2][1] + a.v[1][3] * b.v[3][1],
			a.v[1][0] * b.v[0][2] + a.v[1][1] * b.v[1][2] + a.v[1][2] * b.v[2][2] + a.v[1][3] * b.v[3][2],
			a.v[1][0] * b.v[0][3] + a.v[1][1] * b.v[1][3] + a.v[1][2] * b.v[2][3] + a.v[1][3] * b.v[3][3],

			a.v[2][0] * b.v[0][0] + a.v[2][1] * b.v[1][0] + a.v[2][2] * b.v[2][0] + a.v[2][3] * b.v[3][0],
			a.v[2][0] * b.v[0][1] + a.v[2][1] * b.v[1][1] + a.v[2][2] * b.v[2][1] + a.v[2][3] * b.v[3][1],
			a.v[2][0] * b.v[0][2] + a.v[2][1] * b.v[1][2] + a.v[2][2] * b.v[2][2] + a.v[2][3] * b.v[3][2],
			a.v[2][0] * b.v[0][3] + a.v[2][1] * b.v[1][3] + a.v[2][2] * b.v[2][3] + a.v[2][3] * b.v[3][3],

			a.v[3][0] * b.v[0][0] + a.v[3][1] * b.v[1][0] + a.v[3][2] * b.v[2][0] + a.v[3][3] * b.v[3][0],
			a.v[3][0] * b.v[0][1] + a.v[3][1] * b.v[1][1] + a.v[3][2] * b.v[2][1] + a.v[3][3] * b.v[3][1],
			a.v[3][0] * b.v[0][2] + a.v[3][1] * b.v[1][2] + a.v[3][2] * b.v[2][2] + a.v[3][3] * b.v[3][2],
			a.v[3][0] * b.v[0][3] + a.v[3][1] * b.v[1][3] + a.v[3][2] * b.v[2][3] + a.v[3][3] * b.v[3][3],
		}
	};

	return c;
}

struct vec4
vec4_normalize(struct vec4 v)
{
	float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	if (mag == 0) {
		mag = 1;
	}

	struct vec4 r = {
		v.x / mag, v.y / mag, v.z / mag
	};

	return r;
}

struct vec4
vec4_add(struct vec4 a, struct vec4 b)
{
	struct vec4 r = {
		a.x + b.x, a.y + b.y, a.z + b.z
	};

	return r;
}

struct vec4
vec4_sub(struct vec4 a, struct vec4 b)
{
	struct vec4 r = {
		a.x - b.x, a.y - b.y, a.z - b.z
	};

	return r;
}

struct vec4
vec4_scale(struct vec4 v, float s)
{
	struct vec4 r = {
		v.x * s, v.y * s, v.z * s
	};

	return r;
}

struct mat4
gen_look_at(struct camera c)
{
	struct vec4 up = { 0, 1, 0 };
	struct vec4 dir = vec4_normalize(c.tgt);
	struct vec4 right = vec4_normalize(vec4_cross(up, dir));
	struct vec4 cup = vec4_cross(dir, right);

	struct mat4 la = {
		.v = {
			right.x, right.y, right.z, 0,
			cup.x, cup.y, cup.z, 0,
			dir.x, dir.y, dir.z, 0,
			0, 0, 0, 1
		}
	};

	return mat4_mult_mat4(la, gen_trans_mat4(vec4_scale(c.pos, -1)));
}

void
print_matrix(struct mat4 m)
{
	printf(
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n"
		"%7.3f %7.3f %7.3f %7.3f\n",
		m.v[0][0], m.v[0][1], m.v[0][2], m.v[0][3],
		m.v[1][0], m.v[1][1], m.v[1][2], m.v[1][3],
		m.v[2][0], m.v[2][1], m.v[2][2], m.v[2][3],
		m.v[3][0], m.v[3][1], m.v[3][2], m.v[3][3]
		);

	printf("---\n");
}
