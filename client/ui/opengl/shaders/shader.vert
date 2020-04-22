#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
out vec4 inclr;
out vec3 normal;
out vec3 frag_pos;

const float heights[] = float[](
	4.8,  //tile_deep_water,
	4.8,  //tile_water,
	5,  //tile_wetland,
	5.1,  //tile_plain,
	6.1,  //tile_forest,
	7.0,  //tile_mountain,
	14.0,  //tile_peak,
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
uniform ivec2 corner;
uniform uint tiles[256];

void main()
{
	mat4 model;

	if (gl_InstanceID == 0) {
		model = mat4(
			16, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 16, 0,
			float(corner.x) + 7.5,
			-0.5,
			float(corner.y) + 7.5,
			1
		);
		inclr = vec4(1.0, 1.0, 1.0, 1.0);
	} else {
		uint tile = tiles[gl_InstanceID - 1];

		model = mat4(
			1, 0, 0, 0,
			0, heights[tile], 0, 0,
			0, 0, 1, 0,
			float(corner.x) + uint(gl_InstanceID - 1) / 16u,
			heights[tile] / 2,
			float(corner.y) + uint(gl_InstanceID - 1) % 16u,
			1

		);

		if (tile <= 1u) {
			inclr = vec4(vec3(tile_color[tile]), 0.3);
		} else {
			inclr = vec4(vec3(tile_color[tile]), 1.0);
		}
	}

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
}
