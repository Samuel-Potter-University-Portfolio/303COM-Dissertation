#version 420 core

in vec3 passUVs;


out vec4 outColour;


layout(binding = 0) uniform samplerCube texSky;


void main()
{
	outColour.rgb = texture(texSky, -passUVs).rgb;
	outColour.a = 1.0;
}