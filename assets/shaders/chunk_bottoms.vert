#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 norm;
layout (location = 2) in float type;

uniform mat4 light_space;
uniform mat4 viewproj;
uniform vec3 clip_plane;

flat out vec4 inclr;

void
main()
{
	float above_water = dot(vertex, vec3(0, 1, 0));

	gl_Position = viewproj * vec4(vertex.x, 0.0, vertex.z, 1);

	gl_ClipDistance[0] = dot(vec3(0, above_water, -above_water), clip_plane);

	inclr = vec4(0);
}
