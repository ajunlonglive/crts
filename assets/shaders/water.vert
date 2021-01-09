#version 330 core

in vec3 position;

out vec4 screen_pos;
out vec2 tex_coords;

uniform mat4 viewproj;

void
main(void)
{
	tex_coords = position.xz * 0.04;
	gl_Position = screen_pos = viewproj * vec4(position, 1.0);
	gl_ClipDistance[0] = 0;
}
