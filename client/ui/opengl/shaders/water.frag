#version 330 core

out vec4 clr;

in vec4 frag_pos;

uniform sampler2D reflect_tex;
uniform sampler2D refract_tex;
uniform sampler2D depth_tex;

void
main()
{
	vec3 proj = (frag_pos.xyz / frag_pos.w) * 0.5 + 0.5;

	vec4 reflect_clr = texture(reflect_tex, vec2(proj.x, -proj.y));
	vec4 refract_clr = texture(refract_tex, proj.xy);

	clr = mix(reflect_clr, refract_clr, 0.5);
}
