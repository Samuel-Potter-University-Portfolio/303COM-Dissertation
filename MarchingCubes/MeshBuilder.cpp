#include "MeshBuilder.h"
#include "Mesh.h"
#include "Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>


uint32 MeshBuilderMinimal::AddVertex(const vec3& vertex, const vec3& normal)
{
	auto it = m_indexLookup.find(vertex);

	// Found vertex entry already
	if (it != m_indexLookup.end())
	{
		m_normals[it->second] += normal; // Smooths normals
		return it->second;
	}


	// Unique entry
	uint32 index = m_vertices.size();
	m_vertices.push_back(vertex);
	m_normals.push_back(normal);
	m_indexLookup[vertex] = index;
	return index;
}

void MeshBuilderMinimal::BuildMesh(Mesh* target) const
{
	if (bIsDynamic)
		target->MarkDynamic();

	target->SetVertices(m_vertices);
	target->SetNormals(m_normals);
	target->SetTriangles(m_indices);
}
