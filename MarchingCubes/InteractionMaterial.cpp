#include "InteractionMaterial.h"
#include "GL/glew.h"


InteractionMaterial::InteractionMaterial()
{
	m_shader = new Shader;

	m_shader->LoadVertexShaderFromFile("Resources/Shaders/interaction.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources/Shaders/interaction.frag.glsl");
	m_shader->LinkShader();

	m_shader->SetCullFace(true);


	m_uniformColour = m_shader->GetUniform("colour");
}

void InteractionMaterial::Bind(const Window* window, const Level* level)
{
	Super::Bind(window, level);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_shader->SetUniformVec4(m_uniformColour, vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void InteractionMaterial::Unbind(const Window* window, const Level* level)
{
	Super::Unbind(window, level);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}