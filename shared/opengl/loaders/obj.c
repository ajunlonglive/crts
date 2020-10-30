#include "posix.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/linalg.h"
#include "shared/opengl/loaders/obj.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

#define MAX_VERTS_PER_FACE 16

struct obj_ctx {
	struct darr *verts, *pos, *indices;
	size_t off;
	float scale;
};

enum vert_type {
	vt_pos,
	vt_texture,
	vt_norm,
};


typedef float vertinfo[6];

/* must remain sorted by prefix length */
enum prefix {
	pre_shadow_obj, // shadow casting
	pre_trace_obj, // ray tracing
	pre_d_interp, // dissolve interpolation
	pre_c_interp, // color interpolation
	pre_comment, // comment
	pre_usemtl, // material name
	pre_mtllib, // material library
	pre_cstype, // rational or non-rational forms of curve or surface type
	pre_stech, // surface approximation technique
	pre_curv2, // 2D curve
	pre_ctech, // curve approximation technique
	pre_bevel, // bevel interpolation
	pre_trim, // outer trimming loop
	pre_surf, // surface
	pre_step, // step size
	pre_scrv, // special curve
	pre_parm, // parameter values
	pre_hole, // inner trimming loop
	pre_curv, // curve
	pre_bmat, // basis matrix
	pre_lod, // level of detail
	pre_deg, // degree
	pre_con, // connect
	pre_vt, // texture vertices
	pre_vp, // parameter space vertices
	pre_vn, // vertex normals
	pre_sp, // special point
	pre_mg, // merging group
	pre_v, // geometric vertices
	pre_s, // smoothing group
	pre_p, // point
	pre_o, // object name
	pre_l, // line
	pre_g, // group name
	pre_f, // face

	prefix_count,
	pre_invalid,
};

static struct { char *s; size_t len; } line_prefixes[] = {
	[pre_v] = "v", [pre_vt] = "vt", [pre_vn] = "vn", [pre_vp] = "vp",
	[pre_cstype] = "cstype", [pre_deg] = "deg", [pre_bmat] = "bmat",
	[pre_step] = "step", [pre_p] = "p", [pre_l] = "l", [pre_f] = "f",
	[pre_curv] = "curv", [pre_curv2] = "curv2", [pre_surf] = "surf",
	[pre_parm] = "parm", [pre_trim] = "trim", [pre_hole] = "hole",
	[pre_scrv] = "scrv", [pre_sp] = "sp", [pre_con] = "con", [pre_g] = "g",
	[pre_s] = "s", [pre_mg] = "mg", [pre_o] = "o", [pre_bevel] = "bevel",
	[pre_c_interp] = "c_interp", [pre_d_interp] = "d_interp",
	[pre_lod] = "lod", [pre_usemtl] = "usemtl", [pre_mtllib] = "mtllib",
	[pre_shadow_obj] = "shadow_obj", [pre_trace_obj] = "trace_obj",
	[pre_ctech] = "ctech", [pre_stech] = "stech", [pre_comment] = "#",
};

void
obj_loader_setup(void)
{
	uint8_t i;

	for (i = 0; i < prefix_count; ++i) {
		line_prefixes[i].len = strlen(line_prefixes[i].s);
	}
}

enum prefix
get_prefix(char *line, char **rem)
{
	uint8_t i;

	for (i = 0; i < prefix_count; ++i) {
		if (strncmp(line, line_prefixes[i].s, line_prefixes[i].len) == 0) {
			*rem = line + line_prefixes[i].len + 1;
			return i;
		}
	}

	return pre_invalid;
}

static void
parse_vertex(char *line, vec3 v)
{
	char *endptr;
	uint8_t i;
	float num;

	assert(v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f);

	for (i = 0; i < 3; ++i) {
		num = strtof(line, &endptr);

		if (line == endptr) {
			break;
		}

		v[i] = num;
		line = endptr;
	}
}

static void
parse_face(struct obj_ctx *ctx, char *line, size_t len)
{
	char *endptr;
	uint64_t n, i = 0, indices[MAX_VERTS_PER_FACE];
	enum vert_type vert_type;
	float pos[6] = { 0 };
	vertinfo *vi;
	vec3 *v;

	while (*line != '\0') {
		while (is_whitespace(*line)) {
			++line;
		}

		endptr = line;

		vert_type = vt_pos;
		while (!is_whitespace(*line)) {
			n = strtoul(line, &endptr, 10);

			if (n == 0) {
				goto skip_num;
			}

			--n; /* .obj vertices are 1-indexed */

			switch (vert_type) {
			case vt_pos:
				v = darr_get(ctx->pos, n);
				pos[0] = (*v)[0] * ctx->scale;
				pos[1] = (*v)[1] * ctx->scale;
				pos[2] = (*v)[2] * ctx->scale;
				break;
			case vt_texture:
				/* skip texture coord */
				break;
			case vt_norm:
#if 0
				/* We manually calculate the normal anyway,
				 * perhaps make this optional */
				v = darr_get(ctx->norm, n);
				ve[3] = (*v)[0];
				ve[4] = (*v)[1];
				ve[5] = (*v)[2];
#endif
				break;
			}

skip_num:
			if (is_whitespace(*endptr)
			    || *endptr == '\0') {
				line = endptr;
				break;
			} else if (*endptr == '/') {
				line = endptr + 1;
			} else {
				LOG_W("invalid seperator: '%c'", *endptr);
				assert(false);
				return;
			}

			++vert_type;
		}

		indices[i] = darr_push(ctx->verts, pos) - ctx->off;

		if (i > 1) {
			darr_push(ctx->indices, &indices[0]);
			darr_push(ctx->indices, &indices[i - 1]);
			darr_push(ctx->indices, &indices[i]);

			vi = darr_raw_memory(ctx->verts);

			calc_normal(
				vi[indices[0]     + ctx->off],
				vi[indices[i - 1] + ctx->off],
				vi[indices[i]     + ctx->off],
				&vi[indices[i]     + ctx->off][3]);
		}

		++i;
		assert(i < MAX_VERTS_PER_FACE);
	}
}

static enum iteration_result
parse_line(void *_ctx, char *line, size_t len)
{
	struct obj_ctx *ctx = _ctx;
	enum prefix pre;
	char *tail;
	vec3 v = { 0 };

	if (*line == '\0') {
		return ir_cont;
	}

	switch (pre = get_prefix(line, &tail)) {
	case pre_invalid:
		LOG_W("invalid line: '%s'", line);
		break;
	case pre_v:
		parse_vertex(tail, v);
		darr_push(ctx->pos, v);
		break;
#if 0
	case pre_vn:
		parse_vertex(tail, v);
		darr_push(ctx->norm, v);
		break;
#endif
	case pre_f:
		parse_face(ctx, tail, len);
		break;
	default:
		/* skip everything else for now */
		break;
	}

	return ir_cont;
}

/* assumes all vertex entries come first, I can't tell if the spec mandates
 * this, but it seems common */
bool
obj_load(char *filename, struct darr *verts, struct darr *indices, float scale)
{
	struct file_data *fd;
	if (!(fd = asset(filename))) {
		return false;
	}

	assert(darr_item_size(verts) == sizeof(vertinfo));
	assert(darr_item_size(indices) == sizeof(uint32_t));

	struct obj_ctx ctx = {
		.verts = verts,
		.indices = indices,
		.pos = darr_init(sizeof(vec3)),
		.off = darr_len(verts),
		.scale = scale //0.0016f
	};

	each_line(fd, &ctx, parse_line);

	darr_destroy(ctx.pos);

	assert(darr_len(ctx.indices) % 3 == 0);
	return true;
}
