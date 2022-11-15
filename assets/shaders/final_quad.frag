#version 330 core

out vec4 clr;

in vec2 tex_coord;

uniform sampler2D world_tex;
uniform sampler2D depth_tex;
uniform vec3 focus;
uniform mat4 inv_view;
uniform mat4 inv_proj;

const float near = 0.1;
const float far = 100.0;
const float focus_range = 100.0;

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(tex_coord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inv_proj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inv_view * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
	vec3 world_pos = WorldPosFromDepth(texture(depth_tex, tex_coord).r);
	float dist = distance(focus, world_pos);
	float blur_linear = clamp(dist, 0, focus_range) / focus_range;
	float blur = clamp(log(blur_linear/2 + 1), 0, 1);

	clr = textureLod(world_tex, tex_coord, blur * 8);
}
