#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec4 clr;

uniform sampler2D fontAtlas;

void main()
{
    FragColor = vec4(texture(fontAtlas, texCoord).xyz, 1.0) * clr;
}
