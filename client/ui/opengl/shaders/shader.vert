#version 330 core

layout (location = 0) in vec3 vertex;
flat out uint tile_type;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model * vec4(vertex, 1.0f);
	tile_type = 0u;
}
