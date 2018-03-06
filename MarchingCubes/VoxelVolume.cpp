#include "VoxelVolume.h"
#include "MarchingCubes.h"

#include <unordered_map>
#include "DefaultMaterial.h"


/**
* The appropriate hashing functions which are needed to use vec3 as a key
* https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
*/
struct vec3_KeyFuncs
{
	inline size_t operator()(const vec3& v)const
	{
		return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
	}

	inline bool operator()(const vec3& a, const vec3& b)const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};



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
	m_mesh->MarkDynamic();

	m_data.LoadFromPvmFile("Resources/Lobster.pvm");	
	BuildMesh();
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

void VoxelVolume::BuildMesh() 
{
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	const float isoLevel = 0.5f;
	vec3 edges[12];

	std::vector<vec3> vertices;
	std::vector<uint32> triangles;

	for (uint32 x = 0; x < m_data.GetWidth() - 1; ++x)
		for (uint32 y = 0; y < m_data.GetHeight() - 1; ++y)
			for (uint32 z = 0; z < m_data.GetDepth() - 1; ++z)
			{
				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (m_data.Get(x + 0, y + 0, z + 0) >= isoLevel) caseIndex |= 1;
				if (m_data.Get(x + 1, y + 0, z + 0) >= isoLevel) caseIndex |= 2;
				if (m_data.Get(x + 1, y + 0, z + 1) >= isoLevel) caseIndex |= 4;
				if (m_data.Get(x + 0, y + 0, z + 1) >= isoLevel) caseIndex |= 8;
				if (m_data.Get(x + 0, y + 1, z + 0) >= isoLevel) caseIndex |= 16;
				if (m_data.Get(x + 1, y + 1, z + 0) >= isoLevel) caseIndex |= 32;
				if (m_data.Get(x + 1, y + 1, z + 1) >= isoLevel) caseIndex |= 64;
				if (m_data.Get(x + 0, y + 1, z + 1) >= isoLevel) caseIndex |= 128;
			

				// Fully inside iso-surface
				if (caseIndex == 0 || caseIndex == 255)
					continue;

				// Smooth edges based on density
				if (MC::CaseRequiredEdges[caseIndex] & 1)
					edges[0] = vec3(x + 0.5f, y + 0.0f, z + 0.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 2)
					edges[1] = vec3(x + 1.0f, y + 0.0f, z + 0.5f);
				if (MC::CaseRequiredEdges[caseIndex] & 4)
					edges[2] = vec3(x + 0.5f, y + 0.0f, z + 1.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 8)
					edges[3] = vec3(x + 0.0f, y + 0.0f, z + 0.5f);
				if (MC::CaseRequiredEdges[caseIndex] & 16)
					edges[4] = vec3(x + 0.5f, y + 1.0f, z + 0.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 32)
					edges[5] = vec3(x + 1.0f, y + 1.0f, z + 0.5f);
				if (MC::CaseRequiredEdges[caseIndex] & 64)
					edges[6] = vec3(x + 0.5f, y + 1.0f, z + 1.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 128)
					edges[7] = vec3(x + 0.0f, y + 1.0f, z + 0.5f);
				if (MC::CaseRequiredEdges[caseIndex] & 256)
					edges[8] = vec3(x + 0.0f, y + 0.5f, z + 0.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 512)
					edges[9] = vec3(x + 1.0f, y + 0.5f, z + 0.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 1024)
					edges[10] = vec3(x + 1.0f, y + 0.5f, z + 1.0f);
				if (MC::CaseRequiredEdges[caseIndex] & 2048)
					edges[11] = vec3(x + 0.0f, y + 0.5f, z + 1.0f);
				// TODO - ACTUALLY SMOOTH

				// Add triangles for this case
				int8* caseEdges = MC::Cases[caseIndex];
				while (*caseEdges != -1)
				{
					int8 edge = *(caseEdges++);
					vec3 vert = edges[edge];


					// Reuse old vertex (Lets us do normal smoothing)
					auto it = vertexIndexLookup.find(vert);
					if (it != vertexIndexLookup.end())
						triangles.emplace_back(it->second);

					else
					{
						const uint32 index = vertices.size();
						triangles.emplace_back(index);
						vertices.emplace_back(vert);

						vertexIndexLookup[vert] = index;
					}

				}
			}


	m_mesh->SetVertices(vertices);
	m_mesh->SetTriangles(triangles);
}