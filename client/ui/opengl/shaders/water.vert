#version 330 core

in vec3 position;

out vec4 frag_pos;

uniform mat4 viewproj;

void
main(void)
{
	frag_pos = viewproj * vec4(position.xyz, 1.0);
	gl_Position = frag_pos;
}
