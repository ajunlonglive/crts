#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 clr;

uniform mat4 viewproj;

flat out vec4 inclr;

void
main()
{
	gl_ClipDistance[0] = 0;
	gl_Position = viewproj * vec4(vertex, 1);
	inclr = vec4(clr, 1);
}
