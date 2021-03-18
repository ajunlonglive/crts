#version 330 core

layout (location = 0) in vec2 in_vertex;
layout (location = 1) in vec4 in_color;

out vec4 color;

uniform mat4 proj;

void main()
{
	gl_Position = proj * vec4(in_vertex, 0.0, 1.0);
	gl_Position.z = 0.7;
	color = in_color;
	gl_ClipDistance[0] = 0;
}
