#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 norm;
layout (location = 2) in float type;

uniform mat4 light_space;
uniform mat4 viewproj;
uniform vec3 clip_plane;

uniform vec4 colors[256];

flat out vec3 normal;
flat out vec4 inclr;
out vec3 frag_pos;
out vec4 frag_pos_light_space;

void
main()
{
	vec4 pos = vec4(vertex, 1.0);

	frag_pos = vertex;

	frag_pos_light_space = light_space * pos;

	gl_Position = viewproj * pos;

	float above_water = dot(pos, vec4(0, 1, 0, 0));
	gl_ClipDistance[0] = dot(vec3(0, above_water, -above_water), clip_plane);

	normal = normalize(norm);

	inclr = colors[uint(type)];
}
