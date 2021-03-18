#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 clr;

out vec4 inclr;

uniform mat4 proj;

void
main()
{
	gl_Position = vec4((proj * vec4(vertex, 1.0)).xy, 0.4, 1.0);
	gl_Position.z = 0.9;
	inclr = vec4(clr, 1.0);
}
