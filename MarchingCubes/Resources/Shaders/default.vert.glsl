#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

out vec3 passPos;
out vec3 passNormal;


void main()
{
	// Don't translate at all
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1.0);
	gl_Position = ViewToClip * WorldToView * worldLocation;
	
	passPos = worldLocation.xyz;
	passNormal = inNormal;
}