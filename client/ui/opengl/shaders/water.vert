#version 330 core

in vec3 position;

out vec3 frag_pos;
out vec4 screen_pos;
out vec2 tex_coords;

uniform mat4 viewproj;
uniform vec3 light_pos;
uniform vec3 view_pos;

void
main(void)
{
	tex_coords = position.xz * 0.04;
	gl_Position = screen_pos = viewproj * vec4(position, 1.0);
	frag_pos  = position.xyz;
}
