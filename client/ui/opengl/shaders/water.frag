#version 330 core

out vec4 clr;

in vec3 frag_pos;
in vec2 tex_coords;
in vec4 screen_pos;

uniform float pulse;
uniform sampler2D depth_tex;
uniform sampler2D reflect_tex;
uniform sampler2D refract_tex;
uniform sampler2D ripple_tex;
uniform sampler2D normal_tex;
uniform vec3 light_pos;
uniform vec3 view_pos;

vec3 lightColor = vec3(1, 1, 1);

void
main()
{
	vec3 proj = (screen_pos.xyz / screen_pos.w) * 0.5 + 0.5;

	vec2 dcoords = texture(ripple_tex, tex_coords + pulse).xy * 0.1 + tex_coords;

	vec4 ripple = texture(ripple_tex, dcoords);
	ripple.y -= 0.5;
	ripple *= 0.1;

	vec3 norm = normalize(vec3(ripple.x, ripple.y * 32, ripple.z) * 2.3 - 1);

	vec3 lightDir = normalize(light_pos - frag_pos);
	vec3 viewDir = normalize(view_pos - frag_pos);
	vec3 reflected = reflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflected), 0.0), 16);
	vec3 specular = lightColor * spec;

	vec4 reflect_clr = texture(reflect_tex, vec2(proj.x, -proj.y) + ripple.xy);
	vec4 refract_clr = texture(refract_tex, proj.xy + ripple.xy);

	clr = mix(reflect_clr, refract_clr, 0.5) + vec4(specular, 1.0);
}
