#include "MeshBuilder.h"
#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>


uint32 MeshBuilderFull::AddVertex(const vec3& vertex, const vec3& normal, const vec4& colour, const vec2& uv)
{
	// Check to see if vertex of same type exists
	VertexEntry entry = { vertex, normal, colour, uv };
	auto it = m_vertexLookup.find(entry);

	// Found vertex entry already
	if (it != m_vertexLookup.end())
		return it->second;


	// Unique entry
	uint32 index = m_vertices.size();
	m_vertices.push_back(vertex);
	m_normals.push_back(normal);
	m_colours.push_back(colour);
	m_uvs.push_back(uv);

	m_vertexLookup[entry] = index;
	return index;
}

void MeshBuilderFull::BuildMesh(Mesh* target) const
{
	if (bIsDynamic)
		target->MarkDynamic();

	target->SetVertices(m_vertices);
	target->SetNormals(m_normals);
	target->SetColours(m_colours);
	target->SetUVs(m_uvs);
	target->SetTriangles(m_triangles);
}

void MeshBuilderFull::SmoothNormals()
{
	// Make normals out of weighted triangles
	std::unordered_map<uint32, vec3> normalLookup;


	// Generate normals from tris
	for (uint32 i = 0; i < m_triangles.size(); i += 3)
	{
		uint32 ai = m_triangles[i];
		uint32 bi = m_triangles[i + 1];
		uint32 ci = m_triangles[i + 2];

		vec3 a = m_vertices[ai];
		vec3 b = m_vertices[bi];
		vec3 c = m_vertices[ci];


		// Normals are weighed based on the angle of the edges that connect that corner
		vec3 crossed = glm::cross(b - a, c - a);
		vec3 normal = glm::normalize(crossed);
		float area = crossed.length() * 0.5f;

		normalLookup[ai] += crossed * glm::angle(b - a, c - a);
		normalLookup[bi] += crossed * glm::angle(a - b, c - b);
		normalLookup[ci] += crossed * glm::angle(a - c, b - c);
	}


	for (uint32 i = 0; i < m_normals.size(); ++i)
		m_normals[i] = glm::normalize(normalLookup[i]);

	// TODO - Re-optimize mesh
}


uint32 MeshBuilderMinimal::AddVertex(const vec3& vertex, const vec3& normal)
{
	// Check to see if vertex of same type exists
	VertexEntry entry = { vertex, normal };
	auto it = m_vertexLookup.find(entry);

	// Found vertex entry already
	if (it != m_vertexLookup.end())
		return it->second;


	// Unique entry
	uint32 index = m_vertices.size();
	m_vertices.push_back(vertex);
	m_normals.push_back(normal);

	m_vertexLookup[entry] = index;
	return index;
}

void MeshBuilderMinimal::BuildMesh(Mesh* target) const
{
	if (bIsDynamic)
		target->MarkDynamic();

	target->SetVertices(m_vertices);
	target->SetNormals(m_normals);
	target->SetTriangles(m_triangles);
}

void MeshBuilderMinimal::SmoothNormals()
{
	// Make normals out of weighted triangles
	std::unordered_map<uint32, vec3> normalLookup;


	// Generate normals from tris
	for (uint32 i = 0; i < m_triangles.size(); i += 3)
	{
		uint32 ai = m_triangles[i];
		uint32 bi = m_triangles[i + 1];
		uint32 ci = m_triangles[i + 2];

		vec3 a = m_vertices[ai];
		vec3 b = m_vertices[bi];
		vec3 c = m_vertices[ci];


		// Normals are weighed based on the angle of the edges that connect that corner
		vec3 crossed = glm::cross(b - a, c - a);
		vec3 normal = glm::normalize(crossed);
		float area = crossed.length() * 0.5f;

		normalLookup[ai] += crossed * glm::angle(b - a, c - a);
		normalLookup[bi] += crossed * glm::angle(a - b, c - b);
		normalLookup[ci] += crossed * glm::angle(a - c, b - c);
	}


	for (uint32 i = 0; i < m_normals.size(); ++i)
		m_normals[i] = glm::normalize(normalLookup[i]);

	// TODO - Re-optimize mesh
}