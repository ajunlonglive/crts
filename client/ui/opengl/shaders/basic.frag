#version 330 core

out vec4 clr;

flat in vec4 inclr;

/* uniform vec3 view_pos; */

void main()
{
	clr = inclr;
}
