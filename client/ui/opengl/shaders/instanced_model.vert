#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 pos_offset;
layout (location = 3) in vec3 color;
layout (location = 4) in float scale;

out vec3 frag_pos;
flat out vec3 normal;
flat out vec4 inclr;

uniform mat4 view;
uniform mat4 proj;

void
main()
{
	mat4 model = mat4(
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		pos_offset.xyz, 1
	);

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
	inclr = vec4(color, 1.0);
}
