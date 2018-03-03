#include "SkyboxMaterial.h"
#include "GL/glew.h"


SkyboxMaterial::SkyboxMaterial()
{
	m_shader = new Shader;

	m_shader->LoadVertexShaderFromFile("Resources/Shaders/skybox.vert.glsl");
	m_shader->LoadFragmentShaderFromFile("Resources/Shaders/skybox.frag.glsl");
	m_shader->LinkShader();


	m_skyTexture = new Texture;
	m_skyTexture->SetSmooth(true);
	m_skyTexture->LoadCubemapFromFiles(
		"Resources/Sky/Back.png",
		"Resources/Sky/Front.png",
		"Resources/Sky/Left.png",
		"Resources/Sky/Right.png",
		"Resources/Sky/Up.png",
		"Resources/Sky/Down.png"
	);
}

SkyboxMaterial::~SkyboxMaterial() 
{
	delete m_skyTexture;
}

void SkyboxMaterial::Bind(const Window* window, const Level* level)
{
	Super::Bind(window, level);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexture->GetID());
}

void SkyboxMaterial::Unbind(const Window* window, const Level* level)
{
	Super::Unbind(window, level);
}
