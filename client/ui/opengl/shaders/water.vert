#version 330 core

in vec3 position;
in vec2 texCoords;

out vec4 frag_pos;
out vec2 tex_coords;

uniform mat4 viewproj;

void
main(void)
{
	tex_coords = texCoords;
	gl_Position = frag_pos = viewproj * vec4(position.xyz, 1.0);
}
