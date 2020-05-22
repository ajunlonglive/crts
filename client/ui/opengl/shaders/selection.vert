#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 norm;

uniform mat4 view;
uniform mat4 proj;

flat out vec4 inclr;
flat out vec3 normal;
out vec3 frag_pos;

void
main()
{
	inclr = vec4(color, 0.8);
	frag_pos = vertex;
	gl_Position = proj * view * vec4(vertex, 1.0);
	normal = normalize(norm);
	gl_PointSize = 9;
}
