#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;


out float passCamDistance;


void main()
{
	vec4 worldLocation = ObjectToWorld * vec4(inPos + (dot(inNormal, inNormal) == 0.0 ? vec3(0,0,0) : normalize(inNormal)) * 0.01, 1.0);
	vec4 viewPos = WorldToView * worldLocation;
	passCamDistance = length(viewPos);
	gl_Position = ViewToClip * viewPos;
}