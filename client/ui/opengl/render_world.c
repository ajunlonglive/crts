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

static const uint16_t chunk_indices_len = 512 * 3;
static const uint16_t chunk_indices[] = {
	18, 1, 0,
	17, 18, 0,
	19, 2, 1,
	18, 19, 1,
	20, 3, 2,
	19, 20, 2,
	21, 4, 3,
	20, 21, 3,
	22, 5, 4,
	21, 22, 4,
	23, 6, 5,
	22, 23, 5,
	24, 7, 6,
	23, 24, 6,
	25, 8, 7,
	24, 25, 7,
	26, 9, 8,
	25, 26, 8,
	27, 10, 9,
	26, 27, 9,
	28, 11, 10,
	27, 28, 10,
	29, 12, 11,
	28, 29, 11,
	30, 13, 12,
	29, 30, 12,
	31, 14, 13,
	30, 31, 13,
	32, 15, 14,
	31, 32, 14,
	33, 16, 15,
	32, 33, 15,
	35, 18, 17,
	34, 35, 17,
	36, 19, 18,
	35, 36, 18,
	37, 20, 19,
	36, 37, 19,
	38, 21, 20,
	37, 38, 20,
	39, 22, 21,
	38, 39, 21,
	40, 23, 22,
	39, 40, 22,
	41, 24, 23,
	40, 41, 23,
	42, 25, 24,
	41, 42, 24,
	43, 26, 25,
	42, 43, 25,
	44, 27, 26,
	43, 44, 26,
	45, 28, 27,
	44, 45, 27,
	46, 29, 28,
	45, 46, 28,
	47, 30, 29,
	46, 47, 29,
	48, 31, 30,
	47, 48, 30,
	49, 32, 31,
	48, 49, 31,
	50, 33, 32,
	49, 50, 32,
	52, 35, 34,
	51, 52, 34,
	53, 36, 35,
	52, 53, 35,
	54, 37, 36,
	53, 54, 36,
	55, 38, 37,
	54, 55, 37,
	56, 39, 38,
	55, 56, 38,
	57, 40, 39,
	56, 57, 39,
	58, 41, 40,
	57, 58, 40,
	59, 42, 41,
	58, 59, 41,
	60, 43, 42,
	59, 60, 42,
	61, 44, 43,
	60, 61, 43,
	62, 45, 44,
	61, 62, 44,
	63, 46, 45,
	62, 63, 45,
	64, 47, 46,
	63, 64, 46,
	65, 48, 47,
	64, 65, 47,
	66, 49, 48,
	65, 66, 48,
	67, 50, 49,
	66, 67, 49,
	69, 52, 51,
	68, 69, 51,
	70, 53, 52,
	69, 70, 52,
	71, 54, 53,
	70, 71, 53,
	72, 55, 54,
	71, 72, 54,
	73, 56, 55,
	72, 73, 55,
	74, 57, 56,
	73, 74, 56,
	75, 58, 57,
	74, 75, 57,
	76, 59, 58,
	75, 76, 58,
	77, 60, 59,
	76, 77, 59,
	78, 61, 60,
	77, 78, 60,
	79, 62, 61,
	78, 79, 61,
	80, 63, 62,
	79, 80, 62,
	81, 64, 63,
	80, 81, 63,
	82, 65, 64,
	81, 82, 64,
	83, 66, 65,
	82, 83, 65,
	84, 67, 66,
	83, 84, 66,
	86, 69, 68,
	85, 86, 68,
	87, 70, 69,
	86, 87, 69,
	88, 71, 70,
	87, 88, 70,
	89, 72, 71,
	88, 89, 71,
	90, 73, 72,
	89, 90, 72,
	91, 74, 73,
	90, 91, 73,
	92, 75, 74,
	91, 92, 74,
	93, 76, 75,
	92, 93, 75,
	94, 77, 76,
	93, 94, 76,
	95, 78, 77,
	94, 95, 77,
	96, 79, 78,
	95, 96, 78,
	97, 80, 79,
	96, 97, 79,
	98, 81, 80,
	97, 98, 80,
	99, 82, 81,
	98, 99, 81,
	100, 83, 82,
	99, 100, 82,
	101, 84, 83,
	100, 101, 83,
	103, 86, 85,
	102, 103, 85,
	104, 87, 86,
	103, 104, 86,
	105, 88, 87,
	104, 105, 87,
	106, 89, 88,
	105, 106, 88,
	107, 90, 89,
	106, 107, 89,
	108, 91, 90,
	107, 108, 90,
	109, 92, 91,
	108, 109, 91,
	110, 93, 92,
	109, 110, 92,
	111, 94, 93,
	110, 111, 93,
	112, 95, 94,
	111, 112, 94,
	113, 96, 95,
	112, 113, 95,
	114, 97, 96,
	113, 114, 96,
	115, 98, 97,
	114, 115, 97,
	116, 99, 98,
	115, 116, 98,
	117, 100, 99,
	116, 117, 99,
	118, 101, 100,
	117, 118, 100,
	120, 103, 102,
	119, 120, 102,
	121, 104, 103,
	120, 121, 103,
	122, 105, 104,
	121, 122, 104,
	123, 106, 105,
	122, 123, 105,
	124, 107, 106,
	123, 124, 106,
	125, 108, 107,
	124, 125, 107,
	126, 109, 108,
	125, 126, 108,
	127, 110, 109,
	126, 127, 109,
	128, 111, 110,
	127, 128, 110,
	129, 112, 111,
	128, 129, 111,
	130, 113, 112,
	129, 130, 112,
	131, 114, 113,
	130, 131, 113,
	132, 115, 114,
	131, 132, 114,
	133, 116, 115,
	132, 133, 115,
	134, 117, 116,
	133, 134, 116,
	135, 118, 117,
	134, 135, 117,
	137, 120, 119,
	136, 137, 119,
	138, 121, 120,
	137, 138, 120,
	139, 122, 121,
	138, 139, 121,
	140, 123, 122,
	139, 140, 122,
	141, 124, 123,
	140, 141, 123,
	142, 125, 124,
	141, 142, 124,
	143, 126, 125,
	142, 143, 125,
	144, 127, 126,
	143, 144, 126,
	145, 128, 127,
	144, 145, 127,
	146, 129, 128,
	145, 146, 128,
	147, 130, 129,
	146, 147, 129,
	148, 131, 130,
	147, 148, 130,
	149, 132, 131,
	148, 149, 131,
	150, 133, 132,
	149, 150, 132,
	151, 134, 133,
	150, 151, 133,
	152, 135, 134,
	151, 152, 134,
	154, 137, 136,
	153, 154, 136,
	155, 138, 137,
	154, 155, 137,
	156, 139, 138,
	155, 156, 138,
	157, 140, 139,
	156, 157, 139,
	158, 141, 140,
	157, 158, 140,
	159, 142, 141,
	158, 159, 141,
	160, 143, 142,
	159, 160, 142,
	161, 144, 143,
	160, 161, 143,
	162, 145, 144,
	161, 162, 144,
	163, 146, 145,
	162, 163, 145,
	164, 147, 146,
	163, 164, 146,
	165, 148, 147,
	164, 165, 147,
	166, 149, 148,
	165, 166, 148,
	167, 150, 149,
	166, 167, 149,
	168, 151, 150,
	167, 168, 150,
	169, 152, 151,
	168, 169, 151,
	171, 154, 153,
	170, 171, 153,
	172, 155, 154,
	171, 172, 154,
	173, 156, 155,
	172, 173, 155,
	174, 157, 156,
	173, 174, 156,
	175, 158, 157,
	174, 175, 157,
	176, 159, 158,
	175, 176, 158,
	177, 160, 159,
	176, 177, 159,
	178, 161, 160,
	177, 178, 160,
	179, 162, 161,
	178, 179, 161,
	180, 163, 162,
	179, 180, 162,
	181, 164, 163,
	180, 181, 163,
	182, 165, 164,
	181, 182, 164,
	183, 166, 165,
	182, 183, 165,
	184, 167, 166,
	183, 184, 166,
	185, 168, 167,
	184, 185, 167,
	186, 169, 168,
	185, 186, 168,
	188, 171, 170,
	187, 188, 170,
	189, 172, 171,
	188, 189, 171,
	190, 173, 172,
	189, 190, 172,
	191, 174, 173,
	190, 191, 173,
	192, 175, 174,
	191, 192, 174,
	193, 176, 175,
	192, 193, 175,
	194, 177, 176,
	193, 194, 176,
	195, 178, 177,
	194, 195, 177,
	196, 179, 178,
	195, 196, 178,
	197, 180, 179,
	196, 197, 179,
	198, 181, 180,
	197, 198, 180,
	199, 182, 181,
	198, 199, 181,
	200, 183, 182,
	199, 200, 182,
	201, 184, 183,
	200, 201, 183,
	202, 185, 184,
	201, 202, 184,
	203, 186, 185,
	202, 203, 185,
	205, 188, 187,
	204, 205, 187,
	206, 189, 188,
	205, 206, 188,
	207, 190, 189,
	206, 207, 189,
	208, 191, 190,
	207, 208, 190,
	209, 192, 191,
	208, 209, 191,
	210, 193, 192,
	209, 210, 192,
	211, 194, 193,
	210, 211, 193,
	212, 195, 194,
	211, 212, 194,
	213, 196, 195,
	212, 213, 195,
	214, 197, 196,
	213, 214, 196,
	215, 198, 197,
	214, 215, 197,
	216, 199, 198,
	215, 216, 198,
	217, 200, 199,
	216, 217, 199,
	218, 201, 200,
	217, 218, 200,
	219, 202, 201,
	218, 219, 201,
	220, 203, 202,
	219, 220, 202,
	222, 205, 204,
	221, 222, 204,
	223, 206, 205,
	222, 223, 205,
	224, 207, 206,
	223, 224, 206,
	225, 208, 207,
	224, 225, 207,
	226, 209, 208,
	225, 226, 208,
	227, 210, 209,
	226, 227, 209,
	228, 211, 210,
	227, 228, 210,
	229, 212, 211,
	228, 229, 211,
	230, 213, 212,
	229, 230, 212,
	231, 214, 213,
	230, 231, 213,
	232, 215, 214,
	231, 232, 214,
	233, 216, 215,
	232, 233, 215,
	234, 217, 216,
	233, 234, 216,
	235, 218, 217,
	234, 235, 217,
	236, 219, 218,
	235, 236, 218,
	237, 220, 219,
	236, 237, 219,
	239, 222, 221,
	238, 239, 221,
	240, 223, 222,
	239, 240, 222,
	241, 224, 223,
	240, 241, 223,
	242, 225, 224,
	241, 242, 224,
	243, 226, 225,
	242, 243, 225,
	244, 227, 226,
	243, 244, 226,
	245, 228, 227,
	244, 245, 227,
	246, 229, 228,
	245, 246, 228,
	247, 230, 229,
	246, 247, 229,
	248, 231, 230,
	247, 248, 230,
	249, 232, 231,
	248, 249, 231,
	250, 233, 232,
	249, 250, 232,
	251, 234, 233,
	250, 251, 233,
	252, 235, 234,
	251, 252, 234,
	253, 236, 235,
	252, 253, 235,
	254, 237, 236,
	253, 254, 236,
	256, 239, 238,
	255, 256, 238,
	257, 240, 239,
	256, 257, 239,
	258, 241, 240,
	257, 258, 240,
	259, 242, 241,
	258, 259, 241,
	260, 243, 242,
	259, 260, 242,
	261, 244, 243,
	260, 261, 243,
	262, 245, 244,
	261, 262, 244,
	263, 246, 245,
	262, 263, 245,
	264, 247, 246,
	263, 264, 246,
	265, 248, 247,
	264, 265, 247,
	266, 249, 248,
	265, 266, 248,
	267, 250, 249,
	266, 267, 249,
	268, 251, 250,
	267, 268, 250,
	269, 252, 251,
	268, 269, 251,
	270, 253, 252,
	269, 270, 252,
	271, 254, 253,
	270, 271, 253,
	273, 256, 255,
	272, 273, 255,
	274, 257, 256,
	273, 274, 256,
	275, 258, 257,
	274, 275, 257,
	276, 259, 258,
	275, 276, 258,
	277, 260, 259,
	276, 277, 259,
	278, 261, 260,
	277, 278, 260,
	279, 262, 261,
	278, 279, 261,
	280, 263, 262,
	279, 280, 262,
	281, 264, 263,
	280, 281, 263,
	282, 265, 264,
	281, 282, 264,
	283, 266, 265,
	282, 283, 265,
	284, 267, 266,
	283, 284, 266,
	285, 268, 267,
	284, 285, 267,
	286, 269, 268,
	285, 286, 268,
	287, 270, 269,
	286, 287, 269,
	288, 271, 270,
	287, 288, 270,
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16_t) * chunk_indices_len, chunk_indices,
		GL_STREAM_DRAW);

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

#define MESH_DIM (CHUNK_SIZE + 1)
static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *ck, *rck, *bck, *cck;
	struct point sp = nearest_chunk(&ctx->ref.pos), adjp;
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height,
	    x, y;
	enum tile t;
	uint16_t i;

	float mesh[MESH_DIM * MESH_DIM][3][3] = { 0 };

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if (!(ck = hdarr_get(cnks->hd, &sp))) {
				continue;
			}

			adjp = sp;
			adjp.x += CHUNK_SIZE;
			rck = hdarr_get(cnks->hd, &adjp);

			adjp.y += CHUNK_SIZE;
			cck = hdarr_get(cnks->hd, &adjp);

			adjp.x -= CHUNK_SIZE;
			bck = hdarr_get(cnks->hd, &adjp);

			for (y = 0; y < MESH_DIM; ++y) {
				for (x = 0; x < MESH_DIM; ++x) {
					if (x >= CHUNK_SIZE && y >= CHUNK_SIZE) {
						t = cck ? cck->tiles[0][0] : 0;
					} else if (x >= CHUNK_SIZE) {
						t = rck ? rck->tiles[0][y] : 0;
					} else if (y >= CHUNK_SIZE && bck) {
						t = bck ? bck->tiles[x][0] : 0;
					} else {
						t = ck->tiles[x][y];
					}

					i = y * MESH_DIM + x;

					mesh[i][0][0] = ck->pos.x + x - 0.5;
					mesh[i][0][1] = heights[t];
					mesh[i][0][2] = ck->pos.y + y - 0.5;
					//L("%d, %d -> %d, %f, %f", x, y, y * MESH_DIM + x, ck->pos.x + x + 0.5, ck->pos.y + y + 0.5);

					mesh[i][1][0] = colors.tile[t][0];
					mesh[i][1][1] = colors.tile[t][1];
					mesh[i][1][2] = colors.tile[t][2];

					mesh[i][2][0] = 0;
					mesh[i][2][1] = 0;
					mesh[i][2][2] = 0;
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

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MESH_DIM * MESH_DIM * 3 * 3,
				mesh, GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, chunk_indices_len,
				GL_UNSIGNED_SHORT, (void *)0);

			//glDrawArrays(GL_POINTS, 0, MESH_DIM * MESH_DIM);
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
