#version 330 core

layout (location = 0) in vec3 vertex;
out vec4 inclr;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec4 tile_color[25];
uniform ivec2 corner;
uniform uint tiles[256];

void main()
{
	uint tile = tiles[gl_InstanceID];

	mat4 model = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		float(corner.x) + uint(gl_InstanceID) / 16u,
		1,
		float(corner.y) + uint(gl_InstanceID) % 16u,
		1
	);

	gl_Position = proj * view * model * vec4(vertex, 1.0);
	inclr = tile_color[tile];
}
