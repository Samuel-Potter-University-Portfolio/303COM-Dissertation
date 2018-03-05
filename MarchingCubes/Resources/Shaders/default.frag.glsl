#version 420 core

in vec3 passPos;


out vec4 outColour;


void main()
{
	outColour.rgb = mod(passPos, 1.0);
	outColour.a = 1.0;
}