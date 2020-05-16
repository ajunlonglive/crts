#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;

out vec3 frag_pos;
out vec3 normal;
out vec4 inclr;

uniform mat4 view;
uniform mat4 proj;

uniform ivec3 positions[256];
uniform uint types[256];

mat4 model;

void
setup_ent(uint id)
{
	uint type = types[id];

	inclr = vec4(0.3, 0.1, 0.5, 1.0);
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
	setup_ent(uint(gl_InstanceID));

	frag_pos = vec3(model * vec4(vertex, 1.0));
	gl_Position = proj * view * model * vec4(vertex, 1.0);
	normal = in_normal;
}
