#include "DefaultMaterial.h"



DefaultMaterial::DefaultMaterial()
{
	m_shader = new Shader;

	m_shader->LoadVertexShaderFromFile("Resources/Shaders/default.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources/Shaders/default.frag.glsl");
	m_shader->LinkShader();
}

DefaultMaterial::~DefaultMaterial()
{
}

void DefaultMaterial::Bind(const Window* window, const Level* level) 
{
	Super::Bind(window, level);
}

void DefaultMaterial::Unbind(const Window* window, const Level* level) 
{
	Super::Unbind(window, level);
}