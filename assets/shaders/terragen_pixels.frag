#version 330 core

out vec4 clr;

in vec2 tex_coord;

uniform sampler2D tex;

void main()
{
	clr = texture(tex, tex_coord);
}
