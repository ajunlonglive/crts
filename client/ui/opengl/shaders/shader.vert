#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
out vec4 inclr;
out vec3 normal;
out vec3 frag_pos;

const float heights[] = float[](
	-0.2,  //tile_deep_water,
	-0.2,  //tile_water,
	0,  //tile_wetland,
	0.1,  //tile_plain,
	1.1,  //tile_forest,
	4.0,  //tile_mountain,
	9.0,  //tile_peak,
	0.1,  //tile_dirt,
	0.5, //tile_forest_young,
	1.1,  //tile_forest_old,
	0.5,  //tile_wetland_forest_young,
	1.1,  //tile_wetland_forest,
	1.1,  //tile_wetland_forest_old,
	-0.2,   //tile_coral,

	1,  //tile_wood,
	1,  //tile_stone,
	0.2,  //tile_wood_floor,
	0.2,  //tile_rock_floor,
	3,  //tile_shrine,
	0,  //tile_farmland_empty,
	0.4,  //tile_farmland_done,
	0,  //tile_burning,
	0.1  //tile_burnt,
);
uniform mat4 view;
uniform mat4 proj;
// TODO: put this in frag shader
uniform vec4 tile_color[25];
uniform ivec2 corner;
uniform uint tiles[256];

void main()
{
	uint tile = tiles[gl_InstanceID];

	mat4 model = mat4(
		1, 0, 0, 0,
		0, 10, 0, 0,
		0, 0, 1, 0,
		float(corner.x) + uint(gl_InstanceID) / 16u,
		heights[tile],
		float(corner.y) + uint(gl_InstanceID) % 16u,
		1
	);

	frag_pos = vec3(model * vec4(vertex, 1.0));

	gl_Position = proj * view * model * vec4(vertex, 1.0);
	inclr = tile_color[tile];
	normal = in_normal;
}
