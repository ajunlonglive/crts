#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;

out vec3 frag_pos;
flat out vec3 normal;
flat out vec4 inclr;

uniform mat4 view;
uniform mat4 proj;

uniform vec3 positions[256];
uniform uint types[256];

mat4 model;

void
setup_ent(uint id)
{
	uint type = types[id];

	inclr = vec4(0.3, 0.1, 0.5, 1.0);

	model = mat4(
		0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		positions[id].xzy, 1
	);
}

void
main()
{
	setup_ent(uint(gl_InstanceID));

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
}
