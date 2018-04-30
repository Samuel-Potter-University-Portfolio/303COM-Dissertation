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

struct uvec2_KeyFuncs
{
	inline size_t operator()(const uvec2& v)const
	{
		return std::hash<uint32>()(v.x) ^ std::hash<uint32>()(v.y);
	}

	inline bool operator()(const uvec2& a, const uvec2& b)const
	{
		return a.x == b.x && a.y == b.y;
	}
};

void MeshBuilderMinimal::PerformEdgeCollapseReduction(const uint32& count) 
{
	// As per https://github.com/andandandand/progressive-mesh-reduction-with-edge-collapse

	std::unordered_map<uint32, std::unordered_map<uvec2, vec3, uvec2_KeyFuncs, uvec2_KeyFuncs>> triNormals;
	std::unordered_map<uvec2, float, uvec2_KeyFuncs, uvec2_KeyFuncs> edgeCosts;
	
	// Produce triangle lists
	for (uint32 i = 0; i < m_indices.size(); i += 3)
	{
		const uint32& a = m_indices[i + 0];
		const uint32& b = m_indices[i + 1];
		const uint32& c = m_indices[i + 2];

		const vec3& A = m_vertices[a];
		const vec3& B = m_vertices[b];
		const vec3& C = m_vertices[c];

		// Add normal
		const vec3 normal = normalize(cross(B - A, C - A));
		triNormals[a][uvec2(b, c)] = normal;
		triNormals[b][uvec2(a, c)] = normal;
		triNormals[c][uvec2(a, b)] = normal;

		// Init with empty costs
		edgeCosts[uvec2(b, a)] = 0.0f;
		edgeCosts[uvec2(c, a)] = 0.0f;

		edgeCosts[uvec2(a, b)] = 0.0f;
		edgeCosts[uvec2(c, b)] = 0.0f;

		edgeCosts[uvec2(a, c)] = 0.0f;
		edgeCosts[uvec2(b, c)] = 0.0f;
	}

	// Calculate costs
	for (auto& edge : edgeCosts)
		{
			// Calc cost for a->b
			const uint32& a = edge.first.x;
			const uint32& b = edge.first.y;
			const float len = length(m_vertices[a] - m_vertices[b]);

			float cost = 0;

			for (auto& triA : triNormals[a])
			{
				float minPart = 10000000000;

				for (auto& triAinner : triNormals[a])
				{
					// Triangle does't contain edge a->b
					if (triAinner.first.x != b && triAinner.first.y != b)
						continue;

					const float inner = (1.0f - dot(triA.second, triAinner.second)) * 0.5f;
					if (inner < minPart)
						minPart = inner;
				}

				if (minPart > cost)
					cost = minPart;
			}

			edge.second = cost;
		}


	std::unordered_map<uint32, uint32> edgeMerges;
	
	// Remove edges
	for (int32 i = 0; i < count; ) 
	{
		float minCost = 10000000000;
		uvec2 minEdge;

		// Find edge with smallest cost
		for(auto& pair : edgeCosts)
			if (pair.second != 0 && pair.second < minCost)
			{
				minCost = pair.second;
				minEdge = pair.first;
			}
		
		// Add merged edge to table
		edgeMerges[minEdge.x] = minEdge.y;
		auto oldTrisA = triNormals[minEdge.x];
		auto oldTrisB = triNormals[minEdge.y];
		int32 removeCount = oldTrisA.size() + oldTrisB.size();

		// Get rid of old tris
		triNormals.erase(minEdge.x);
		triNormals.erase(minEdge.y);
		edgeCosts.erase(minEdge);

		for (auto it = edgeCosts.begin(); it != edgeCosts.end();) 
		{
			if (it->first.x == minEdge.x || it->first.y == minEdge.x)
				edgeCosts.erase(it++);
			else
				it++;
		}


		// Add new tris
		std::vector<uvec2> newEdges;

		for (auto& tri : oldTrisB)
		{
			// Safe to re-add
			if (tri.first.x != minEdge.x && tri.first.y != minEdge.x)
			{
				triNormals[minEdge.y][tri.first] = tri.second;
				newEdges.push_back(uvec2(minEdge.y, tri.first.x));
				newEdges.push_back(uvec2(minEdge.y, tri.first.y));

				newEdges.push_back(uvec2(tri.first.x, minEdge.y));
				newEdges.push_back(uvec2(tri.first.x, tri.first.y));

				newEdges.push_back(uvec2(tri.first.y, minEdge.y));
				newEdges.push_back(uvec2(tri.first.y, tri.first.x));
				removeCount--;
			}
		}
		for (auto& tri : oldTrisA)
		{
			// Safe to re-add
			if (tri.first.x != minEdge.y && tri.first.y != minEdge.y)
			{
				auto it = triNormals[minEdge.y].find(tri.first);

				if (it == triNormals[minEdge.y].end())
				{
					triNormals[minEdge.y][tri.first] = tri.second;
					newEdges.push_back(uvec2(minEdge.y, tri.first.x));
					newEdges.push_back(uvec2(minEdge.y, tri.first.y));

					newEdges.push_back(uvec2(tri.first.x, minEdge.y));
					newEdges.push_back(uvec2(tri.first.x, tri.first.y));

					newEdges.push_back(uvec2(tri.first.y, minEdge.y));
					newEdges.push_back(uvec2(tri.first.y, tri.first.x));
					removeCount--;
				}
			}
		}

		// Re-calc costs
		for (const vec2& edge : newEdges)
		{
			// Calc cost for a->b
			const uint32& a = edge.x;
			const uint32& b = edge.y;
			const float len = length(m_vertices[a] - m_vertices[b]);

			float cost = 0;

			for (auto& triA : triNormals[a])
			{
				float minPart = 10000000000;

				for (auto& triAinner : triNormals[a])
				{
					// Triangle does't contain edge a->b
					if (triAinner.first.x != b && triAinner.first.y != b)
						continue;

					const float inner = (1.0f - dot(triA.second, triAinner.second)) * 0.5f;
					if (inner < minPart)
						minPart = inner;
				}

				if (minPart > cost)
					cost = minPart;
			}

			edgeCosts[edge] = cost;
		}

		if (removeCount == 0)
			i++;
		else
			i += removeCount;
	}

	// Put new edges into mesh
	std::vector<vec3> oldVertices = m_vertices;
	std::vector<vec3> oldNormals = m_normals;
	std::vector<uint32> oldIndices = m_indices;

	m_vertices.clear();
	m_normals.clear();
	m_indices.clear();
	m_indexLookup.clear();

	for (uint32 i = 0; i < oldIndices.size(); i += 3) 
	{
		uint32 a = oldIndices[i + 0];
		uint32 b = oldIndices[i + 1];
		uint32 c = oldIndices[i + 2];

		auto it = edgeMerges.find(a);
		if (it != edgeMerges.end())
			a = it->second;
		it = edgeMerges.find(b);
		if (it != edgeMerges.end())
			b = it->second;
		it = edgeMerges.find(c);
		if (it != edgeMerges.end())
			c = it->second;

		const vec3& A = oldVertices[a];
		const vec3& B = oldVertices[b];
		const vec3& C = oldVertices[c];

		if (A != B && A != C && B != C)
		{
			const uint32 na = AddVertex(A, oldNormals[a]);
			const uint32 nb = AddVertex(B, oldNormals[b]);
			const uint32 nc = AddVertex(C, oldNormals[c]);
			AddTriangle(na, nb, nc);
		}
	}
}
