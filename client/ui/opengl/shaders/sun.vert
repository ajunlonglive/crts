#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 viewproj;

void
main(void) {
	gl_Position =  viewproj * vec4(pos, 1.0);
	gl_ClipDistance[0] = 0;
}
