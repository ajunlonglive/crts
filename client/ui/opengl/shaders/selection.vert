#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 color;

out float gl_ClipDistance[1];
uniform float pulse;
uniform mat4 viewproj;

flat out vec4 inclr;

void
main()
{
	vec4 pos = vec4(vertex, 1.0);

	float br = (cos(pulse * 15) + 1) * 0.5;

	inclr = vec4(color * br, 0.8);

	gl_Position = viewproj * pos;

	gl_ClipDistance[0] = dot(pos, vec4(0, 1, 0, 0));
}
