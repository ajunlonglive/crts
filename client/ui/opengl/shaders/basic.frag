#version 330 core

out vec4 clr;

flat in vec4 inclr;

/* TODO make some default uniforms in shader.c optional
 *
 * we don't use view_pos, so it gets optimized out and we get an error
 */

/* uniform vec3 view_pos; */

void main()
{
	clr = inclr;
}
