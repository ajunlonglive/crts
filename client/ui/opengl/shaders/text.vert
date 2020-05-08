#version 330 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 texPos;

out vec2 texCoord;
out vec3 clr;

uniform vec2 atlasCoords[256];
uniform uint string[256];
uniform vec2 charDims;
uniform vec2 iniPos;
uniform mat4 proj;

void main()
{
	float pos = float(gl_InstanceID);

	gl_Position = proj * vec4(vertex.x + iniPos.x + pos, vertex.y + iniPos.y, 0.0, 1.0);

	texCoord = texPos * charDims + atlasCoords[string[gl_InstanceID]];
	clr = vec3(0.0, 1.0, 0.0);
}
