#version 330 core

layout (location = 0) in vec2 in_vertex;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec2 in_vertex_off;
layout (location = 3) in vec2 in_tex_coord_off;
layout (location = 4) in vec4 in_color;

out vec2 tex_coord;
out vec4 color;

uniform mat4 proj;

void main()
{
	gl_Position = proj * vec4((in_vertex + in_vertex_off), -0.1, 1.0);
	tex_coord = in_tex_coord + in_tex_coord_off;
	color = in_color;
	gl_ClipDistance[0] = 0;
}
