#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 pos_offset;
layout (location = 3) in float scale;
layout (location = 4) in vec3 color;

uniform mat4 viewproj;
uniform mat4 light_space;

flat out vec3 normal;
flat out vec4 inclr;
out float gl_ClipDistance[2];
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

	gl_ClipDistance[0] = dot(pos, vec4(0, 1, 0, 0));
	gl_ClipDistance[1] = -gl_ClipDistance[0];

	normal = in_normal;

	inclr = vec4(color, 1.0);
}
