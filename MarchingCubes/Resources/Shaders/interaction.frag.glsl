#version 420 core


uniform vec4 colour;

in float passCamDistance;

out vec4 outColour;


void main()
{
	//if(passCamDistance > 20.5f)
	//	discard;

	outColour = colour;
}