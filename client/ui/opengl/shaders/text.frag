#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 clr;

uniform sampler2D fontAtlas;

void main()
{
    FragColor = texture(fontAtlas, texCoord) * vec4(clr, 1.0) + vec4(0.0, 0.0, 0.0, 0.5);
}
