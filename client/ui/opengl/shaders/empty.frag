#version 330 core

out vec4 clr;

void main()
{
	clr = vec4(gl_FragCoord.xyz, 1.0);
}

