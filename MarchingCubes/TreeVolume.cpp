#include "TreeVolume.h"
#include "Logger.h"
#include "glm.hpp"
#include "DefaultMaterial.h"
#include "VolumeOctree.h"


TreeVolume::TreeVolume()
{
	m_isoLevel = 0.15f;
}

TreeVolume::~TreeVolume()
{
	if (m_material != nullptr)
		delete m_material;
	if (m_octree != nullptr)
		delete m_octree;
}


///
/// Object functions
///

#include "InteractionMaterial.h"
void TreeVolume::Begin()
{
	m_material = new DefaultMaterial;
	//m_material = new InteractionMaterial;

	m_mesh = new Mesh;
	m_mesh->MarkDynamic();

	BuildMesh();
}

void TreeVolume::Update(const float & deltaTime)
{
}

void TreeVolume::Draw(const Window * window, const float & deltaTime)
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


///
/// Object functions
///

void TreeVolume::Init(const uvec3 & resolution, const vec3 & scale, float defaultValue)
{
	// Use even square resolution
	const uint32 max = glm::max(resolution.x, glm::max(resolution.y, resolution.z));
	const uint32 res = pow(2, ceil(log(max) / log(2)));


	LOG("TreeVolume uses power of 2 uniform resolution");
	LOG("(%i,%i,%i) converted to %ix", resolution.x, resolution.y, resolution.z, res);

	m_resolution = uvec3(res, res, res);
	m_scale = scale;
	m_octree = new VolumeOctree(res);
}

void TreeVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	m_octree->Set(x, y, z, value);
}

float TreeVolume::Get(uint32 x, uint32 y, uint32 z)
{
	return m_octree->Get(x, y, z);
}



#include "MarchingCubes.h"
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>

static void MeshFunc(std::vector<vec3>& vertices, std::vector<uint32>& triangles, OctreeVolumeNode* node, vec3 offset)
{
	const auto count = vertices.size();
	vertices.push_back(offset + vec3(0, 0, 0) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(1, 0, 0) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(0, 0, 1) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(1, 0, 1) * (float)node->GetResolution());

	vertices.push_back(offset + vec3(0, 1, 0) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(1, 1, 0) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(0, 1, 1) * (float)node->GetResolution());
	vertices.push_back(offset + vec3(1, 1, 1) * (float)node->GetResolution());

	triangles.push_back(count + 0);
	triangles.push_back(count + 2);
	triangles.push_back(count + 1);
	triangles.push_back(count + 2);
	triangles.push_back(count + 3);
	triangles.push_back(count + 1);

	triangles.push_back(count + 4);
	triangles.push_back(count + 5);
	triangles.push_back(count + 6);
	triangles.push_back(count + 5);
	triangles.push_back(count + 7);
	triangles.push_back(count + 6);

	triangles.push_back(count + 2);
	triangles.push_back(count + 6);
	triangles.push_back(count + 3);
	triangles.push_back(count + 6);
	triangles.push_back(count + 7);
	triangles.push_back(count + 3);

	triangles.push_back(count + 3);
	triangles.push_back(count + 7);
	triangles.push_back(count + 1);
	triangles.push_back(count + 1);
	triangles.push_back(count + 7);
	triangles.push_back(count + 5);

	triangles.push_back(count + 1);
	triangles.push_back(count + 4);
	triangles.push_back(count + 0);
	triangles.push_back(count + 1);
	triangles.push_back(count + 5);
	triangles.push_back(count + 4);

	triangles.push_back(count + 0);
	triangles.push_back(count + 4);
	triangles.push_back(count + 2);
	triangles.push_back(count + 2);
	triangles.push_back(count + 4);
	triangles.push_back(count + 6);

	if (node->GetDepth() == 7)
		return;


	OctreeVolumeBranch* branch = dynamic_cast<OctreeVolumeBranch*>(node);
	if (branch)
	{
		if (branch->c000)
			MeshFunc(vertices, triangles, branch->c000, offset + vec3(0.0f, 0.0f, 0.0f) * (float)node->GetResolution());
		if (branch->c001)
			MeshFunc(vertices, triangles, branch->c001, offset + vec3(0.0f, 0.0f, 0.5f) * (float)node->GetResolution());
		if (branch->c010)
			MeshFunc(vertices, triangles, branch->c010, offset + vec3(0.0f, 0.5f, 0.0f) * (float)node->GetResolution());
		if (branch->c011)
			MeshFunc(vertices, triangles, branch->c011, offset + vec3(0.0f, 0.5f, 0.5f) * (float)node->GetResolution());

		if (branch->c100)
			MeshFunc(vertices, triangles, branch->c100, offset + vec3(0.5f, 0.0f, 0.0f) * (float)node->GetResolution());
		if (branch->c101)
			MeshFunc(vertices, triangles, branch->c101, offset + vec3(0.5f, 0.0f, 0.5f) * (float)node->GetResolution());
		if (branch->c110)
			MeshFunc(vertices, triangles, branch->c110, offset + vec3(0.5f, 0.5f, 0.0f) * (float)node->GetResolution());
		if (branch->c111)
			MeshFunc(vertices, triangles, branch->c111, offset + vec3(0.5f, 0.5f, 0.5f) * (float)node->GetResolution());
	}
}


void TreeVolume::BuildMesh()
{
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	vec3 edges[12];

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<uint32> triangles;

	/*
	for (uint32 x = 0; x < GetResolution().x; ++x)
		for (uint32 y = 0; y < GetResolution().y; ++y)
			for (uint32 z = 0; z < GetResolution().z; ++z)
			{
				if (Get(x, y, z) > m_isoLevel)
				{
					const auto count = vertices.size();
					const vec3 offset(x, y, z);
					vertices.push_back(offset + vec3(0, 0, 0));
					vertices.push_back(offset + vec3(1, 0, 0));
					vertices.push_back(offset + vec3(0, 0, 1));
					vertices.push_back(offset + vec3(1, 0, 1));

					vertices.push_back(offset + vec3(0, 1, 0));
					vertices.push_back(offset + vec3(1, 1, 0));
					vertices.push_back(offset + vec3(0, 1, 1));
					vertices.push_back(offset + vec3(1, 1, 1));

					triangles.push_back(count + 0);
					triangles.push_back(count + 2);
					triangles.push_back(count + 1);
					triangles.push_back(count + 2);
					triangles.push_back(count + 3);
					triangles.push_back(count + 1);

					triangles.push_back(count + 4);
					triangles.push_back(count + 5);
					triangles.push_back(count + 6);
					triangles.push_back(count + 5);
					triangles.push_back(count + 7);
					triangles.push_back(count + 6);

					triangles.push_back(count + 2);
					triangles.push_back(count + 6);
					triangles.push_back(count + 3);
					triangles.push_back(count + 6);
					triangles.push_back(count + 7);
					triangles.push_back(count + 3);

					triangles.push_back(count + 3);
					triangles.push_back(count + 7);
					triangles.push_back(count + 1);
					triangles.push_back(count + 1);
					triangles.push_back(count + 7);
					triangles.push_back(count + 5);

					triangles.push_back(count + 1);
					triangles.push_back(count + 4);
					triangles.push_back(count + 0);
					triangles.push_back(count + 1);
					triangles.push_back(count + 5);
					triangles.push_back(count + 4);

					triangles.push_back(count + 0);
					triangles.push_back(count + 4);
					triangles.push_back(count + 2);
					triangles.push_back(count + 2);
					triangles.push_back(count + 4);
					triangles.push_back(count + 6);
				}
			}*/

	//MeshFunc(vertices, triangles, m_octree->GetRootNode(), vec3(0,0,0));
	//m_mesh->SetVertices(vertices);
	//m_mesh->SetTriangles(triangles);
	//return;


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
					edges[0] = VERT_LERP(0, 0, 0, 1, 0, 0);
				if (MC::CaseRequiredEdges[caseIndex] & 2)
					edges[1] = VERT_LERP(1, 0, 0, 1, 0, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 4)
					edges[2] = VERT_LERP(0, 0, 1, 1, 0, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 8)
					edges[3] = VERT_LERP(0, 0, 0, 0, 0, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 16)
					edges[4] = VERT_LERP(0, 1, 0, 1, 1, 0);
				if (MC::CaseRequiredEdges[caseIndex] & 32)
					edges[5] = VERT_LERP(1, 1, 0, 1, 1, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 64)
					edges[6] = VERT_LERP(0, 1, 1, 1, 1, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 128)
					edges[7] = VERT_LERP(0, 1, 0, 0, 1, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 256)
					edges[8] = VERT_LERP(0, 0, 0, 0, 1, 0);
				if (MC::CaseRequiredEdges[caseIndex] & 512)
					edges[9] = VERT_LERP(1, 0, 0, 1, 1, 0);
				if (MC::CaseRequiredEdges[caseIndex] & 1024)
					edges[10] = VERT_LERP(1, 0, 1, 1, 1, 1);
				if (MC::CaseRequiredEdges[caseIndex] & 2048)
					edges[11] = VERT_LERP(0, 0, 1, 0, 1, 1);


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