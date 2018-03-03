#version 420 core

// Transform matrix
uniform mat4 ObjectToWorld;
// View matrix
uniform mat4 WorldToView;
// Perspective matrix
uniform mat4 ViewToClip;


layout (location = 0) in vec3 inPos;

out vec3 passUVs;


void main()
{
	// Don't translate at all
	vec4 worldLocation = ObjectToWorld * vec4(inPos * 700, 0);
	gl_Position = ViewToClip * vec4((WorldToView * worldLocation).xyz, 1);

	passUVs = inPos.xyz;
}