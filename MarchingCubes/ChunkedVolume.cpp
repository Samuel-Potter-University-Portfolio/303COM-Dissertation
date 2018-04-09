#include "ChunkedVolume.h"
#include "DefaultMaterial.h"
#include "Logger.h"

#include <unordered_map>
#include "MarchingCubes.h"


VoxelChunk::VoxelChunk(const uvec3& offset, uint32 resolution, class ChunkedVolume* parent)
{
	m_offset = offset;
	m_resolution = resolution;
	m_parent = parent;

	mesh = new Mesh;
	mesh->MarkDynamic();
}
VoxelChunk::~VoxelChunk() 
{
	if (mesh != nullptr)
		delete mesh;
}


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>



/**
* Interpolate between voxels to get more precise intersection
* @param isoValue				The value being used currently by MC
* @param a						The position of the first voxel
* @param b						The position of the second voxel
* @param aVal					The value at the first voxel
* @param bVal					The value at the second voxel
*/
static vec3 VertexLerp(float isoLevel, const vec3& a, const vec3& b, float aVal, float bVal)
{
	const float closeValue = 0.00001f;

	if (glm::abs(isoLevel - aVal) < closeValue)
		return a;
	if (glm::abs(isoLevel - bVal) < closeValue)
		return b;
	if (glm::abs(aVal - bVal) < closeValue)
		return a;

	float mu = (isoLevel - aVal) / (bVal - aVal);
	return a + mu * (b - a);
}

void VoxelChunk::BuildMesh()
{
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	vec3 edges[12];
	const float isoLevel = m_parent->GetIsoLevel();

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<uint32> triangles;

	for (uint32 xi = 0; xi < m_resolution; ++xi)
		for (uint32 yi = 0; yi < m_resolution; ++yi)
			for (uint32 zi = 0; zi < m_resolution; ++zi)
			{
				uint32 x = m_offset.x + xi;
				uint32 y = m_offset.y + yi;
				uint32 z = m_offset.z + zi;

				if (x >= m_parent->GetResolution().x - 1 || y >= m_parent->GetResolution().y - 1 || z >= m_parent->GetResolution().z - 1)
					continue;

				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (m_parent->Get(x + 0, y + 0, z + 0) >= isoLevel) caseIndex |= 1;
				if (m_parent->Get(x + 1, y + 0, z + 0) >= isoLevel) caseIndex |= 2;
				if (m_parent->Get(x + 1, y + 0, z + 1) >= isoLevel) caseIndex |= 4;
				if (m_parent->Get(x + 0, y + 0, z + 1) >= isoLevel) caseIndex |= 8;
				if (m_parent->Get(x + 0, y + 1, z + 0) >= isoLevel) caseIndex |= 16;
				if (m_parent->Get(x + 1, y + 1, z + 0) >= isoLevel) caseIndex |= 32;
				if (m_parent->Get(x + 1, y + 1, z + 1) >= isoLevel) caseIndex |= 64;
				if (m_parent->Get(x + 0, y + 1, z + 1) >= isoLevel) caseIndex |= 128;


				// Fully inside iso-surface
				if (caseIndex == 0 || caseIndex == 255)
					continue;

				// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) VertexLerp(isoLevel, vec3(x + x0,y + y0,z + z0), vec3(x + x1, y + y1, z + z1), m_parent->Get(x + x0, y + y0, z + z0), m_parent->Get(x + x1, y + y1, z + z1))

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


	mesh->SetVertices(vertices);
	mesh->SetNormals(normals);
	mesh->SetTriangles(triangles);
	bRequiresRebuild = false;
}


ChunkedVolume::ChunkedVolume() : chunkSize(16)
{
	m_isoLevel = 0.15f;
}
ChunkedVolume::~ChunkedVolume()
{
	for (VoxelChunk* chunk : m_chunks)
		if(chunk != nullptr)
			delete chunk;
	if (m_material != nullptr)
		delete m_material;
	if (m_data != nullptr)
		delete[] m_data;
}


///
/// Volume Data Overrides
///
void ChunkedVolume::Init(const uvec3& resolution, const vec3& scale)
{
	m_data = new float[resolution.x * resolution.y * resolution.z]{ DEFAULT_VALUE };
	m_resolution = resolution;
	m_scale = scale;

	// Ensure vector can hold all possilbe chunks
	m_chunks.resize(
		ceilf(resolution.x / (float)chunkSize) *
		ceilf(resolution.y / (float)chunkSize) *
		ceilf(resolution.z / (float)chunkSize)
	);
		
	LOG("Initialized volume with %i potential chunks of size", m_chunks.size(), chunkSize);
}

void ChunkedVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	m_data[GetVoxelIndex(x, y, z)] = value;
	
	// Make chunk if not made and flag for rebuild
	const uint32 index = GetChunkIndex(x, y, z);
	uvec3 coord = GetChunkCoords(x, y, z);
	VoxelChunk* chunk = m_chunks[index];

	// Create chunk if not already there
	if (chunk == nullptr)
	{
		chunk = new VoxelChunk(uvec3(coord.x * chunkSize, coord.y * chunkSize, coord.z * chunkSize), chunkSize, this);
		m_chunks[index] = chunk;
	}
	chunk->bRequiresRebuild = true;


	// Make adjacent chunks rebuild, if on border
	uint32 xc = x % chunkSize;
	if (xc == 0)
	{
		int i = GetChunkIndex(x - 1, y, z);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}
	else if (xc == chunkSize - 1)
	{
		int i = GetChunkIndex(x + 1, y, z);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}

	uint32 yc = y % chunkSize;
	if (yc == 0)
	{
		int i = GetChunkIndex(x, y - 1, z);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}
	else if (yc == chunkSize - 1)
	{
		int i = GetChunkIndex(x, y + 1, z);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}

	uint32 zc = z % chunkSize;
	if (zc == 0)
	{
		int i = GetChunkIndex(x, y, z - 1);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}
	else if (zc == chunkSize - 1)
	{
		int i = GetChunkIndex(x, y, z + 1);
		if (i >= 0 && i < m_chunks.size() && m_chunks[i] != nullptr)
			m_chunks[i]->bRequiresRebuild = true;
	}
}

float ChunkedVolume::Get(uint32 x, uint32 y, uint32 z)
{
	return m_data[GetVoxelIndex(x, y, z)];
}


///
/// Object functions
///

void ChunkedVolume::Begin()
{
	m_material = new DefaultMaterial;
}

void ChunkedVolume::Update(const float& deltaTime)
{
	// Rebuild mesh if it needs it
	for (VoxelChunk* chunk : m_chunks)
		if (chunk != nullptr && chunk->bRequiresRebuild)
			chunk->BuildMesh();
}


void ChunkedVolume::Draw(const class Window* window, const float& deltaTime)
{
	m_material->Bind(window, GetLevel());
	Transform t;

	for (VoxelChunk* chunk : m_chunks) 
	{
		if (chunk != nullptr && chunk->mesh != nullptr)
		{
			m_material->PrepareMesh(chunk->mesh);
			m_material->RenderInstance(&t);
		}
	}

	m_material->Unbind(window, GetLevel());
}
