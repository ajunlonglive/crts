#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 color;

uniform mat4 viewproj;
uniform float pulse;

flat out vec4 inclr;

void
main()
{
	float br = (cos(pulse * 15) + 1) * 0.5;

	inclr = vec4(color * br, 0.8);
	gl_Position = viewproj * vec4(vertex, 1.0);
}
