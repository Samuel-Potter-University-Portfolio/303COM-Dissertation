#version 420 core

in vec3 passPos;
in vec3 passNormal;


const vec3 lightDirection0 = normalize(vec3(0, -1, 0));
const vec3 lightDirection1 = normalize(vec3(-1, 0, -1));

out vec4 outColour;


void main()
{
	// Calculate Lighting
	vec3 normal = normalize(passNormal);
	
	float diffuse0 = max(dot(-lightDirection0, normal), 0.2);


	vec3 colour = vec3(1, 1, 1);
	outColour.rgb = colour * (diffuse0);
	outColour.a = 1.0;
}