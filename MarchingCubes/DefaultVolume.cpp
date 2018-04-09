#include "DefaultVolume.h"
#include "MarchingCubes.h"

#include <unordered_map>
#include "DefaultMaterial.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>


DefaultVolume::DefaultVolume()
{
	m_isoLevel = 0.15f;
}

DefaultVolume::~DefaultVolume()
{
	if (m_data != nullptr)
		delete[] m_data;
	if (m_mesh != nullptr)
		delete m_mesh;
	if (m_material != nullptr)
		delete m_material;
}


///
/// Volume Data Overrides
///
void DefaultVolume::Init(const uvec3& resolution, const vec3& scale)
{
	m_data = new float[resolution.x * resolution.y * resolution.z]{ DEFAULT_VALUE };
	m_resolution = resolution;
	m_scale = scale;
}

void DefaultVolume::Set(uint32 x, uint32 y, uint32 z, float value) 
{
	m_data[GetIndex(x, y, z)] = value;
	bRequiresRebuild = true;
}

float DefaultVolume::Get(uint32 x, uint32 y, uint32 z) 
{
	return m_data[GetIndex(x, y, z)];
}


///
/// Object functions
///

void DefaultVolume::Begin() 
{
	m_material = new DefaultMaterial;

	m_mesh = new Mesh;
	m_mesh->MarkDynamic();

	BuildMesh();
}

void DefaultVolume::Update(const float& deltaTime) 
{
	// Rebuild mesh if it needs it
	if (bRequiresRebuild)
	{
		BuildMesh();
		bRequiresRebuild = false;
	}
}

void DefaultVolume::Draw(const class Window* window, const float& deltaTime) 
{
	if (m_mesh != nullptr)
	{
		m_material->Bind(window, GetLevel());
		m_material->PrepareMesh(m_mesh);

		Transform t;
		// TODO - Support raycasting into scaling
		//t.SetScale(m_data.GetScale());
		m_material->RenderInstance(&t);

		m_material->Unbind(window, GetLevel());
	}
}

void DefaultVolume::BuildMesh() 
{
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	vec3 edges[12];

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<uint32> triangles;

	for (uint32 x = 0; x < GetResolution().x - 1; ++x)
		for (uint32 y = 0; y < GetResolution().y - 1; ++y)
			for (uint32 z = 0; z < GetResolution().z - 1; ++z)
			{
				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (Get(x + 0, y + 0, z + 0) >= m_isoLevel) caseIndex |= 1;
				if (Get(x + 1, y + 0, z + 0) >= m_isoLevel) caseIndex |= 2;
				if (Get(x + 1, y + 0, z + 1) >= m_isoLevel) caseIndex |= 4;
				if (Get(x + 0, y + 0, z + 1) >= m_isoLevel) caseIndex |= 8;
				if (Get(x + 0, y + 1, z + 0) >= m_isoLevel) caseIndex |= 16;
				if (Get(x + 1, y + 1, z + 0) >= m_isoLevel) caseIndex |= 32;
				if (Get(x + 1, y + 1, z + 1) >= m_isoLevel) caseIndex |= 64;
				if (Get(x + 0, y + 1, z + 1) >= m_isoLevel) caseIndex |= 128;
			

				// Fully inside iso-surface
				if (caseIndex == 0 || caseIndex == 255)
					continue;

				// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(m_isoLevel, vec3(x + x0,y + y0,z + z0), vec3(x + x1, y + y1, z + z1), Get(x + x0, y + y0, z + z0), Get(x + x1, y + y1, z + z1))
				
				if (MC::CaseRequiredEdges[caseIndex] & 1)
					edges[0] = VERT_LERP(0,0,0, 1,0,0);
				if (MC::CaseRequiredEdges[caseIndex] & 2)
					edges[1] = VERT_LERP(1,0,0, 1,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 4)
					edges[2] = VERT_LERP(0,0,1, 1,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 8)
					edges[3] = VERT_LERP(0,0,0, 0,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 16)
					edges[4] = VERT_LERP(0,1,0, 1,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 32)
					edges[5] = VERT_LERP(1,1,0, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 64)
					edges[6] = VERT_LERP(0,1,1, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 128)
					edges[7] = VERT_LERP(0,1,0, 0,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 256)
					edges[8] = VERT_LERP(0,0,0, 0,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 512)
					edges[9] = VERT_LERP(1,0,0, 1,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 1024)
					edges[10] = VERT_LERP(1,0,1, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 2048)
					edges[11] = VERT_LERP(0,0,1, 0,1,1);


				// Add triangles for this case
				int8* caseEdges = MC::Cases[caseIndex];
				while (*caseEdges != -1)
				{
					int8 edge = *(caseEdges++);
					vec3 vert = edges[edge];


					// Reuse old vertex (Lets us do normal smoothing)
					auto it = vertexIndexLookup.find(vert);
					if (it != vertexIndexLookup.end())
					{
						triangles.emplace_back(it->second);
					}
					else
					{
						const uint32 index = vertices.size();
						triangles.emplace_back(index);
						vertices.emplace_back(vert);

						vertexIndexLookup[vert] = index;
					}

				}
			}


	// Make normals out of weighted triangles
	std::unordered_map<uint32, vec3> normalLookup;

	// Generate normals from triss
	for (uint32 i = 0; i < triangles.size(); i += 3)
	{
		uint32 ai = triangles[i];
		uint32 bi = triangles[i + 1];
		uint32 ci = triangles[i + 2];

		vec3 a = vertices[ai];
		vec3 b = vertices[bi];
		vec3 c = vertices[ci];


		// Normals are weighed based on the angle of the edges that connect that corner
		vec3 crossed = glm::cross(b - a, c - a);
		vec3 normal = glm::normalize(crossed);
		float area = crossed.length() * 0.5f;

		normalLookup[ai] += crossed * glm::angle(b - a, c - a);
		normalLookup[bi] += crossed * glm::angle(a - b, c - b);
		normalLookup[ci] += crossed * glm::angle(a - c, b - c);
	}

	// Put normals into vector
	normals.reserve(vertices.size());
	for (uint32 i = 0; i < vertices.size(); ++i)
		normals.emplace_back(normalLookup[i]);


	m_mesh->SetVertices(vertices);
	m_mesh->SetNormals(normals);
	m_mesh->SetTriangles(triangles);
}