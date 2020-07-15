#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat4 light_space;

void
main()
{
	gl_Position = light_space * vec4(vertex, 1.0);
	gl_ClipDistance[0] = 0;
}
