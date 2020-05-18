#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render_world.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

static const uint16_t chunk_indices_len = 1350;
static const uint8_t chunk_indices[] = {
	1, 0, 17,
	0, 16, 17,
	2, 1, 18,
	1, 17, 18,
	3, 2, 19,
	2, 18, 19,
	4, 3, 20,
	3, 19, 20,
	5, 4, 21,
	4, 20, 21,
	6, 5, 22,
	5, 21, 22,
	7, 6, 23,
	6, 22, 23,
	8, 7, 24,
	7, 23, 24,
	9, 8, 25,
	8, 24, 25,
	10, 9, 26,
	9, 25, 26,
	11, 10, 27,
	10, 26, 27,
	12, 11, 28,
	11, 27, 28,
	13, 12, 29,
	12, 28, 29,
	14, 13, 30,
	13, 29, 30,
	15, 14, 31,
	14, 30, 31,
	17, 16, 33,
	16, 32, 33,
	18, 17, 34,
	17, 33, 34,
	19, 18, 35,
	18, 34, 35,
	20, 19, 36,
	19, 35, 36,
	21, 20, 37,
	20, 36, 37,
	22, 21, 38,
	21, 37, 38,
	23, 22, 39,
	22, 38, 39,
	24, 23, 40,
	23, 39, 40,
	25, 24, 41,
	24, 40, 41,
	26, 25, 42,
	25, 41, 42,
	27, 26, 43,
	26, 42, 43,
	28, 27, 44,
	27, 43, 44,
	29, 28, 45,
	28, 44, 45,
	30, 29, 46,
	29, 45, 46,
	31, 30, 47,
	30, 46, 47,
	33, 32, 49,
	32, 48, 49,
	34, 33, 50,
	33, 49, 50,
	35, 34, 51,
	34, 50, 51,
	36, 35, 52,
	35, 51, 52,
	37, 36, 53,
	36, 52, 53,
	38, 37, 54,
	37, 53, 54,
	39, 38, 55,
	38, 54, 55,
	40, 39, 56,
	39, 55, 56,
	41, 40, 57,
	40, 56, 57,
	42, 41, 58,
	41, 57, 58,
	43, 42, 59,
	42, 58, 59,
	44, 43, 60,
	43, 59, 60,
	45, 44, 61,
	44, 60, 61,
	46, 45, 62,
	45, 61, 62,
	47, 46, 63,
	46, 62, 63,
	49, 48, 65,
	48, 64, 65,
	50, 49, 66,
	49, 65, 66,
	51, 50, 67,
	50, 66, 67,
	52, 51, 68,
	51, 67, 68,
	53, 52, 69,
	52, 68, 69,
	54, 53, 70,
	53, 69, 70,
	55, 54, 71,
	54, 70, 71,
	56, 55, 72,
	55, 71, 72,
	57, 56, 73,
	56, 72, 73,
	58, 57, 74,
	57, 73, 74,
	59, 58, 75,
	58, 74, 75,
	60, 59, 76,
	59, 75, 76,
	61, 60, 77,
	60, 76, 77,
	62, 61, 78,
	61, 77, 78,
	63, 62, 79,
	62, 78, 79,
	65, 64, 81,
	64, 80, 81,
	66, 65, 82,
	65, 81, 82,
	67, 66, 83,
	66, 82, 83,
	68, 67, 84,
	67, 83, 84,
	69, 68, 85,
	68, 84, 85,
	70, 69, 86,
	69, 85, 86,
	71, 70, 87,
	70, 86, 87,
	72, 71, 88,
	71, 87, 88,
	73, 72, 89,
	72, 88, 89,
	74, 73, 90,
	73, 89, 90,
	75, 74, 91,
	74, 90, 91,
	76, 75, 92,
	75, 91, 92,
	77, 76, 93,
	76, 92, 93,
	78, 77, 94,
	77, 93, 94,
	79, 78, 95,
	78, 94, 95,
	81, 80, 97,
	80, 96, 97,
	82, 81, 98,
	81, 97, 98,
	83, 82, 99,
	82, 98, 99,
	84, 83, 100,
	83, 99, 100,
	85, 84, 101,
	84, 100, 101,
	86, 85, 102,
	85, 101, 102,
	87, 86, 103,
	86, 102, 103,
	88, 87, 104,
	87, 103, 104,
	89, 88, 105,
	88, 104, 105,
	90, 89, 106,
	89, 105, 106,
	91, 90, 107,
	90, 106, 107,
	92, 91, 108,
	91, 107, 108,
	93, 92, 109,
	92, 108, 109,
	94, 93, 110,
	93, 109, 110,
	95, 94, 111,
	94, 110, 111,
	97, 96, 113,
	96, 112, 113,
	98, 97, 114,
	97, 113, 114,
	99, 98, 115,
	98, 114, 115,
	100, 99, 116,
	99, 115, 116,
	101, 100, 117,
	100, 116, 117,
	102, 101, 118,
	101, 117, 118,
	103, 102, 119,
	102, 118, 119,
	104, 103, 120,
	103, 119, 120,
	105, 104, 121,
	104, 120, 121,
	106, 105, 122,
	105, 121, 122,
	107, 106, 123,
	106, 122, 123,
	108, 107, 124,
	107, 123, 124,
	109, 108, 125,
	108, 124, 125,
	110, 109, 126,
	109, 125, 126,
	111, 110, 127,
	110, 126, 127,
	113, 112, 129,
	112, 128, 129,
	114, 113, 130,
	113, 129, 130,
	115, 114, 131,
	114, 130, 131,
	116, 115, 132,
	115, 131, 132,
	117, 116, 133,
	116, 132, 133,
	118, 117, 134,
	117, 133, 134,
	119, 118, 135,
	118, 134, 135,
	120, 119, 136,
	119, 135, 136,
	121, 120, 137,
	120, 136, 137,
	122, 121, 138,
	121, 137, 138,
	123, 122, 139,
	122, 138, 139,
	124, 123, 140,
	123, 139, 140,
	125, 124, 141,
	124, 140, 141,
	126, 125, 142,
	125, 141, 142,
	127, 126, 143,
	126, 142, 143,
	129, 128, 145,
	128, 144, 145,
	130, 129, 146,
	129, 145, 146,
	131, 130, 147,
	130, 146, 147,
	132, 131, 148,
	131, 147, 148,
	133, 132, 149,
	132, 148, 149,
	134, 133, 150,
	133, 149, 150,
	135, 134, 151,
	134, 150, 151,
	136, 135, 152,
	135, 151, 152,
	137, 136, 153,
	136, 152, 153,
	138, 137, 154,
	137, 153, 154,
	139, 138, 155,
	138, 154, 155,
	140, 139, 156,
	139, 155, 156,
	141, 140, 157,
	140, 156, 157,
	142, 141, 158,
	141, 157, 158,
	143, 142, 159,
	142, 158, 159,
	145, 144, 161,
	144, 160, 161,
	146, 145, 162,
	145, 161, 162,
	147, 146, 163,
	146, 162, 163,
	148, 147, 164,
	147, 163, 164,
	149, 148, 165,
	148, 164, 165,
	150, 149, 166,
	149, 165, 166,
	151, 150, 167,
	150, 166, 167,
	152, 151, 168,
	151, 167, 168,
	153, 152, 169,
	152, 168, 169,
	154, 153, 170,
	153, 169, 170,
	155, 154, 171,
	154, 170, 171,
	156, 155, 172,
	155, 171, 172,
	157, 156, 173,
	156, 172, 173,
	158, 157, 174,
	157, 173, 174,
	159, 158, 175,
	158, 174, 175,
	161, 160, 177,
	160, 176, 177,
	162, 161, 178,
	161, 177, 178,
	163, 162, 179,
	162, 178, 179,
	164, 163, 180,
	163, 179, 180,
	165, 164, 181,
	164, 180, 181,
	166, 165, 182,
	165, 181, 182,
	167, 166, 183,
	166, 182, 183,
	168, 167, 184,
	167, 183, 184,
	169, 168, 185,
	168, 184, 185,
	170, 169, 186,
	169, 185, 186,
	171, 170, 187,
	170, 186, 187,
	172, 171, 188,
	171, 187, 188,
	173, 172, 189,
	172, 188, 189,
	174, 173, 190,
	173, 189, 190,
	175, 174, 191,
	174, 190, 191,
	177, 176, 193,
	176, 192, 193,
	178, 177, 194,
	177, 193, 194,
	179, 178, 195,
	178, 194, 195,
	180, 179, 196,
	179, 195, 196,
	181, 180, 197,
	180, 196, 197,
	182, 181, 198,
	181, 197, 198,
	183, 182, 199,
	182, 198, 199,
	184, 183, 200,
	183, 199, 200,
	185, 184, 201,
	184, 200, 201,
	186, 185, 202,
	185, 201, 202,
	187, 186, 203,
	186, 202, 203,
	188, 187, 204,
	187, 203, 204,
	189, 188, 205,
	188, 204, 205,
	190, 189, 206,
	189, 205, 206,
	191, 190, 207,
	190, 206, 207,
	193, 192, 209,
	192, 208, 209,
	194, 193, 210,
	193, 209, 210,
	195, 194, 211,
	194, 210, 211,
	196, 195, 212,
	195, 211, 212,
	197, 196, 213,
	196, 212, 213,
	198, 197, 214,
	197, 213, 214,
	199, 198, 215,
	198, 214, 215,
	200, 199, 216,
	199, 215, 216,
	201, 200, 217,
	200, 216, 217,
	202, 201, 218,
	201, 217, 218,
	203, 202, 219,
	202, 218, 219,
	204, 203, 220,
	203, 219, 220,
	205, 204, 221,
	204, 220, 221,
	206, 205, 222,
	205, 221, 222,
	207, 206, 223,
	206, 222, 223,
	209, 208, 225,
	208, 224, 225,
	210, 209, 226,
	209, 225, 226,
	211, 210, 227,
	210, 226, 227,
	212, 211, 228,
	211, 227, 228,
	213, 212, 229,
	212, 228, 229,
	214, 213, 230,
	213, 229, 230,
	215, 214, 231,
	214, 230, 231,
	216, 215, 232,
	215, 231, 232,
	217, 216, 233,
	216, 232, 233,
	218, 217, 234,
	217, 233, 234,
	219, 218, 235,
	218, 234, 235,
	220, 219, 236,
	219, 235, 236,
	221, 220, 237,
	220, 236, 237,
	222, 221, 238,
	221, 237, 238,
	223, 222, 239,
	222, 238, 239,
	225, 224, 241,
	224, 240, 241,
	226, 225, 242,
	225, 241, 242,
	227, 226, 243,
	226, 242, 243,
	228, 227, 244,
	227, 243, 244,
	229, 228, 245,
	228, 244, 245,
	230, 229, 246,
	229, 245, 246,
	231, 230, 247,
	230, 246, 247,
	232, 231, 248,
	231, 247, 248,
	233, 232, 249,
	232, 248, 249,
	234, 233, 250,
	233, 249, 250,
	235, 234, 251,
	234, 250, 251,
	236, 235, 252,
	235, 251, 252,
	237, 236, 253,
	236, 252, 253,
	238, 237, 254,
	237, 253, 254,
	239, 238, 255,
	238, 254, 255,
};

static struct {
	uint32_t id;
	uint32_t vao, vbo;
	uint32_t view, proj, view_pos, positions, types;
	struct hash *h;
} s_ent = { 0 };

static bool
render_world_setup_ents(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/ents.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_ent.id)) {
		return false;
	}

	s_ent.view      = glGetUniformLocation(s_ent.id, "view");
	s_ent.proj      = glGetUniformLocation(s_ent.id, "proj");
	s_ent.positions = glGetUniformLocation(s_ent.id, "positions");
	s_ent.types     = glGetUniformLocation(s_ent.id, "types");
	s_ent.view_pos  = glGetUniformLocation(s_ent.id, "view_pos");

	s_ent.h = hash_init(2048, 1, sizeof(struct point));

	glGenVertexArrays(1, &s_ent.vao);
	glGenBuffers(1, &s_ent.vbo);

	glBindVertexArray(s_ent.vao);
	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * solid_cube.len,
		solid_cube.verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return true;
}

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos;
} s_chunk = { 0 };

bool
render_world_setup_chunks(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/chunks.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_chunk.id)) {
		return false;
	}
	s_chunk.view      = glGetUniformLocation(s_chunk.id, "view");
	s_chunk.proj      = glGetUniformLocation(s_chunk.id, "proj");
	s_chunk.view_pos  = glGetUniformLocation(s_chunk.id, "view_pos");

	glGenVertexArrays(1, &s_chunk.vao);
	glGenBuffers(1, &s_chunk.vbo);
	glGenBuffers(1, &s_chunk.ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	glBindVertexArray(s_chunk.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	return true;
}

bool
render_world_setup(char *graphics_path)
{
	return render_world_setup_ents()
	       && render_world_setup_chunks()
	       && color_cfg(graphics_path);
}

void
render_world_teardown(void)
{
	hash_destroy(s_ent.h);
}

void
update_world_viewport(mat4 mproj)
{
	glUseProgram(s_ent.id);
	glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)mproj);

	glUseProgram(s_chunk.id);
	glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)mproj);
}


const float heights[] = {
	4.8,  //tile_deep_water,
	4.8,  //tile_water,
	5,  //tile_wetland,
	5.1,  //tile_plain,
	6.4,  //tile_forest,
	9.3,  //tile_mountain,
	11.3,  //tile_peak,
	5.1,  //tile_dirt,
	5.5, //tile_forest_young,
	6.1,  //tile_forest_old,
	5.5,  //tile_wetland_forest_young,
	6.1,  //tile_wetland_forest,
	6.1,  //tile_wetland_forest_old,
	4.8,   //tile_coral,

	6,  //tile_wood,
	6,  //tile_stone,
	5.2,  //tile_wood_floor,
	5.2,  //tile_rock_floor,
	8,  //tile_shrine,
	5,  //tile_farmland_empty,
	5.4,  //tile_farmland_done,
	5,  //tile_burning,
	5.1  //tile_burnt,
};

static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *ck;
	struct point sp = nearest_chunk(&ctx->ref.pos);
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height,
	    x, y;

	float mesh[256][3][3] = { 0 };

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_indices_len,
		chunk_indices, GL_STREAM_DRAW);

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if (!(ck = hdarr_get(cnks->hd, &sp))) {
				continue;
			}

			for (y = 0; y < CHUNK_SIZE; ++y) {
				for (x = 0; x < CHUNK_SIZE; ++x) {
					mesh[y * 16 + x][0][0] = ck->pos.x + x + 0.5;
					mesh[y * 16 + x][0][1] = heights[ck->tiles[x][y]];
					mesh[y * 16 + x][0][2] = ck->pos.y + y + 0.5;

					mesh[y * 16 + x][1][0] = colors.tile[ck->tiles[x][y]][0];
					mesh[y * 16 + x][1][1] = colors.tile[ck->tiles[x][y]][1];
					mesh[y * 16 + x][1][2] = colors.tile[ck->tiles[x][y]][2];

					mesh[y * 16 + x][2][0] = 0;
					mesh[y * 16 + x][2][1] = 0;
					mesh[y * 16 + x][2][2] = 0;
				}
			}

			vec4 a = { 0 }, b = { 0 }, c = { 0 };

			for (x = 3; x < chunk_indices_len; x += 6) {
				memcpy(a, mesh[chunk_indices[x + 0]][0], sizeof(float) * 3);
				memcpy(b, mesh[chunk_indices[x + 1]][0], sizeof(float) * 3);
				memcpy(c, mesh[chunk_indices[x + 2]][0], sizeof(float) * 3);

				vec4_sub(b, a);
				vec4_sub(c, a);
				vec4_cross(b, c);

				memcpy(mesh[chunk_indices[x + 2]][2], b, sizeof(float) * 3);
			}

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * 16 * 3 * 3,
				mesh, GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, chunk_indices_len,
				GL_UNSIGNED_BYTE, (void *)0);

			//glDrawArrays(GL_POINTS, 0, 256);
		}
	}
}

static void
render_ents(struct hdarr *ents, struct hdarr *cnks, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(ents));
	size_t i, j, len = hdarr_len(ents);

	hash_clear(s_ent.h);

	int32_t positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	const size_t *st;

	for (i = 0, j = 0; i < len; ++i, ++j) {
		if (i >= 256) {
			glUniform1uiv(s_ent.types, 256, types);
			glUniform3iv(s_ent.positions, 256, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256);
			j = 0;
		}

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;

		if ((st = hash_get(s_ent.h, &emem[i].pos))) {
			positions[(j * 3) + 2] = *st + 1;
			hash_set(s_ent.h, &emem[i].pos, *st + 1);
		} else {
			positions[(j * 3) + 2] = 0;
			hash_set(s_ent.h, &emem[i].pos, 0);
		}

		types[j] = emem[i].type;
	}

	glUniform1uiv(s_ent.types, 256, types);
	glUniform3iv(s_ent.positions, 256, positions);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, len % 256);
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	float w, h;

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		L("recalculating pos");

		ctx->ref.pos = hf->view;

		if (cam.unlocked) {
			w = 128;
			h = 128;
		} else {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height / 2;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w * 0.5;
			cam.pos[2] = ctx->ref.pos.y + h * 0.5;
		}

		ctx->ref.width = w;
		ctx->ref.height = h;

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);
		cam.changed = true;
	}

	/* chunks */

	glUseProgram(s_chunk.id);
	glBindVertexArray(s_chunk.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_chunk.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_chunk.view_pos, 1, cam.pos);
	}

	render_chunks(hf->sim->w->chunks, ctx);

	/* ents */

	glUseProgram(s_ent.id);
	glBindVertexArray(s_ent.vao);

	if (cam.changed) {
		glUniformMatrix4fv(s_ent.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_ent.view_pos, 1, cam.pos);
	}

	render_ents(hf->sim->w->ents, hf->sim->w->chunks->hd, ctx);

	cam.changed = false;
}
