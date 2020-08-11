#version 330 core

out vec4 out_color;

in vec2 tex_coord;
in vec4 color;

uniform sampler2D font_atlas;

void main()
{
    out_color = vec4(texture(font_atlas, tex_coord).xyz, 1.0) * color;
}
