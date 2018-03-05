#include "VoxelVolume.h"
#include "DefaultMaterial.h"


VoxelVolume::VoxelVolume()
{
}

VoxelVolume::~VoxelVolume()
{
}

void VoxelVolume::Begin() 
{
	m_material = new DefaultMaterial;

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

	m_mesh->SetNormals(std::vector<vec3>(
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
		0,1,2, 2,1,3,
		4,6,5, 5,6,7,

		2,3,6, 6,3,7,
		3,1,7, 1,5,7,

		1,0,4, 1,4,5,
		0,2,4, 2,6,4,
	}));
}

void VoxelVolume::Update(const float& deltaTime) 
{
}

void VoxelVolume::Draw(const class Window* window, const float& deltaTime) 
{
	if (m_mesh != nullptr)
	{
		m_material->Bind(window, GetLevel());
		m_material->PrepareMesh(m_mesh);
		m_material->RenderInstance(nullptr);
		m_material->Unbind(window, GetLevel());
	}
}