#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;


void main()
{
	// Don't translate at all
	vec4 worldLocation = ObjectToWorld * vec4(inPos, 1.0);
	gl_Position = ViewToClip * WorldToView * worldLocation;
}