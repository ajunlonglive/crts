#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
out vec4 inclr;
out vec3 normal;
out vec3 frag_pos;
flat out uint selected;

const float heights[] = float[](
	4.8,  //tile_deep_water,
	4.8,  //tile_water,
	5,  //tile_wetland,
	5.1,  //tile_plain,
	6.4,  //tile_forest,
	7.3,  //tile_mountain,
	8.3,  //tile_peak,
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
);
uniform mat4 view;
uniform mat4 proj;
// TODO: put this in frag shader
uniform vec4 tile_color[25];
uniform ivec3 positions[256];
uniform uint types[256];
uniform uint bases[256];
uniform uint cat;
uniform ivec2 sel;

mat4 model;

void
setup_chunk_base()
{
	inclr = vec4(1.0, 1.0, 1.0, 1.0);

	model = mat4(
		16, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 16, 0,
		float(positions[0].x) + 7.5,
		-0.5,
		float(positions[0].y) + 7.5,
		1
	);
}

void
setup_chunk_tile(uint id)
{
	uint type = types[id];

	if (type <= 1u) {
		inclr = vec4(vec3(tile_color[type]), 0.3);
	} else {
		inclr = vec4(vec3(tile_color[type]), 1.0);
	}

	ivec2 pos = ivec2(positions[0].x + int(id) / 16, positions[0].y + int(id) % 16);

	if (abs(pos.x - sel.x) < 0.1 && abs(pos.y - sel.y) < 0.1) {
		selected = 1u;
	} else {
		selected = 0u;
	}

	model = mat4(
		1, 0, 0, 0,
		0, heights[type], 0, 0,
		0, 0, 1, 0,
		float(pos.x), heights[type] / 2, float(pos.y), 1
	);
}

void
setup_ent(uint id)
{
	uint type = types[id];
	uint base = bases[id];

	inclr = vec4(0.3, 0.1, 0.5, 0.3);
	int corner = positions[id].z % 4;

	float x = float(positions[id].x) - 0.25;
	float y = float(positions[id].y) - 0.25;
	float z = 5.5 + (0.5 * (positions[id].z / 4));

	switch (corner) {
	case 0:
		break;
	case 1:
		x += 0.5;
		break;
	case 2:
		y += 0.5;
		break;
	case 3:
		x += 0.5;
		y += 0.5;
		break;
	}

	model = mat4(
		0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		x, z, y, 1
	);
}

void
main()
{
	switch (cat) {
	case 0u:
		if (gl_InstanceID == 0) {
			setup_chunk_base();
		} else {
			setup_chunk_tile(uint(gl_InstanceID - 1));
		}
		break;
	case 1u:
		selected = 0u;
		setup_ent(uint(gl_InstanceID));
		break;
	}

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
}
