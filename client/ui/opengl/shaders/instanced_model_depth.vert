#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 pos_offset;
layout (location = 2) in float scale;

uniform mat4 light_space;

void
main()
{
	mat4 model = mat4(
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		pos_offset.xyz, 1
	);

	gl_Position = light_space * model * vec4(vertex, 1.0);
}
