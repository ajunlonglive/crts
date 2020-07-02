#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 norm;
layout (location = 2) in float type;

uniform mat4 viewproj;

uniform vec4 colors[256];

flat out vec4 inclr;
flat out vec3 normal;
out vec3 frag_pos;

void
main()
{
	inclr = colors[uint(type)];
	frag_pos = vertex;
	gl_Position = viewproj * vec4(vertex, 1.0);
	normal = normalize(norm);
}
