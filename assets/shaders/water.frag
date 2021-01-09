#version 330 core

out vec4 clr;

in vec2 tex_coords;
in vec4 screen_pos;

uniform float pulse;
uniform sampler2D reflect_tex;
uniform sampler2D refract_tex;
uniform sampler2D ripple_tex;

void
main()
{
	vec3 proj = (screen_pos.xyz / screen_pos.w) * 0.5 + 0.5;
	vec2 dcoords = texture(ripple_tex, tex_coords + pulse).xy * 0.1 + tex_coords;

	vec4 ripple = vec4(((texture(ripple_tex, dcoords).gb * 2) - 1), 0.0, 1.0);
	vec3 norm = normalize(vec3(ripple.x, ripple.y, ripple.z) * 2.3 - 1);
	ripple *= 0.01;

	float diff = max(dot(norm, vec3(-2.0, 4.0, -2.0)), 0.0) * 0.0001;

	vec2 reflect_coords = vec2(proj.x, -proj.y) + ripple.xy;
	reflect_coords.x = clamp(reflect_coords.x, 0.001, 0.999);
	reflect_coords.y = clamp(reflect_coords.y, -0.999, -0.001);

	vec4 reflect_clr = texture(reflect_tex, reflect_coords);
	vec4 refract_clr = texture(refract_tex, clamp(proj.xy + ripple.xy, 0.001, 0.999));

	clr = mix(reflect_clr, refract_clr, 0.5);
}
