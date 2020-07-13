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

float near = 0.1;
float far = 1000.0;

float
linearize(float val) {
    return 2.0 * near * far / (far + near - (2.0 * val - 1.0) * (far - near));
}

void
main()
{
	vec3 proj = (screen_pos.xyz / screen_pos.w) * 0.5 + 0.5;
	float depth = (linearize(texture(depth_tex, proj.xy).r) - linearize(gl_FragCoord.z)) / far;
	vec2 dcoords = texture(ripple_tex, tex_coords + pulse).xy * 0.1 + tex_coords;

	vec4 ripple = vec4(((texture(ripple_tex, dcoords).gb * 2) - 1), 0.0, 1.0);
	vec3 norm = normalize(vec3(ripple.x, ripple.y, ripple.z) * 2.3 - 1);
	ripple *=  1.0 * clamp(depth * 10, 0.0, 1.0);

	vec3 lightDir = normalize(light_pos - frag_pos);
	vec3 viewDir = normalize(view_pos - frag_pos);
	vec3 reflected = reflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflected), 0.0), 16);
	vec3 specular = lightColor * spec;

	vec2 reflect_coords = vec2(proj.x, -proj.y) + ripple.xy;
	reflect_coords.x = clamp(reflect_coords.x, 0.001, 0.999);
	reflect_coords.y = clamp(reflect_coords.y, -0.999, -0.001);

	vec4 reflect_clr = texture(reflect_tex, reflect_coords);
	vec4 refract_clr = texture(refract_tex, clamp(proj.xy + ripple.xy, 0.001, 0.999));

	clr = mix(reflect_clr, refract_clr, 0.5) + vec4(specular, 1.0);
	/* clr = vec4(texture(refract_tex, proj.xy + ripple.xy).xyz, 1.0); */
}
