#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 pos_offset;
layout (location = 3) in float scale;
layout (location = 4) in vec3 color;

uniform mat4 light_space;
uniform mat4 viewproj;
uniform vec3 clip_plane;

flat out vec3 normal;
flat out vec4 inclr;
out vec3 frag_pos;
out vec4 frag_pos_light_space;

void
main()
{
	mat4 model = mat4(
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		pos_offset.xyz, 1
	);

	vec4 pos = model * vec4(vertex, 1.0);

	frag_pos = pos.xyz;

	frag_pos_light_space = light_space * pos;

	gl_Position =  viewproj * pos;

	float above_water = dot(pos, vec4(0, 1, 0, 0));
	gl_ClipDistance[0] = dot(vec3(0, above_water, -above_water), clip_plane);

	normal = in_normal;

	inclr = vec4(color, 1.0);
}
