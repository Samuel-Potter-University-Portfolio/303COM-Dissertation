#include "InteractionMaterial.h"
#include "GL/glew.h"


InteractionMaterial::InteractionMaterial(const vec4& colour)
{
	m_shader = new Shader;

	m_shader->LoadVertexShaderFromFile("Resources/Shaders/interaction.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources/Shaders/interaction.frag.glsl");
	m_shader->LinkShader();

	m_shader->SetCullFace(true);


	m_uniformColour = m_shader->GetUniform("colour");
	m_colour = colour;
}

void InteractionMaterial::Bind(const Window* window, const Level* level)
{
	Super::Bind(window, level);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_shader->SetUniformVec4(m_uniformColour, m_colour);
}

void InteractionMaterial::Unbind(const Window* window, const Level* level)
{
	Super::Unbind(window, level);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}