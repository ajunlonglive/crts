#version 330 core

out vec4 clr;

in vec4 frag_pos;

uniform sampler2D reflect_tex;

void
main()
{
	vec2 proj = (frag_pos.xy / frag_pos.w) * 0.5 + 0.5;

	clr = vec4(texture(reflect_tex, vec2(proj.x, -proj.y)).xyz, 0.5);

	/* clr = vec4(proj.xy, 1.0, 1.0); */
}
