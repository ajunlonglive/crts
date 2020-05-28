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

uniform vec4 colors[256];

void
main()
{
	uint type = types[gl_InstanceID];

	mat4 model = mat4(
		0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		positions[gl_InstanceID].xzy, 1
	);

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
	inclr = vec4(colors[type].xyz, 1.0);
}
