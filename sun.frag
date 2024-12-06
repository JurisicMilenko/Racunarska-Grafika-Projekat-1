#version 330 core

out vec4 outCol;

uniform float uA;

void main()
{
	outCol = vec4(1.0, uA, 0.0, 1.0);
}