#include "SkyBox.h"



SkyBox::SkyBox()
{
}

SkyBox::~SkyBox()
{
	if(m_mesh != nullptr)
		delete m_mesh;
	if (m_material != nullptr)
		delete m_material;
}

void SkyBox::Begin() 
{
	m_mesh = new Mesh;
	m_mesh->SetVertices(std::vector<vec3>(
	{
		vec3(-1.0f, -1.0f, -1.0f),
		vec3(1.0f, -1.0f, -1.0f),
		vec3(-1.0f, -1.0f, 1.0f),
		vec3(1.0f, -1.0f, 1.0f),

		vec3(-1.0f, 1.0f, -1.0f),
		vec3(1.0f, 1.0f, -1.0f),
		vec3(-1.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),
	}));
	m_mesh->SetTriangles(std::vector<uint32>(
	{
		0,2,1, 2,3,1,
		4,5,6, 5,7,6,

		2,6,3, 6,7,3,
		3,7,1, 1,7,5,

		1,4,0, 1,5,4,
		0,4,2, 2,4,6,

	}));

	m_material = new SkyboxMaterial;
}

void SkyBox::Draw(const Window* window, const float& deltaTime) 
{
	m_material->Bind(window, GetLevel());
	m_material->PrepareMesh(m_mesh);
	m_material->RenderInstance(nullptr);
	m_material->Unbind(window, GetLevel());
}