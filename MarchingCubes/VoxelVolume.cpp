#include "VoxelVolume.h"
#include "MarchingCubes.h"

#include <unordered_map>
#include "DefaultMaterial.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>


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



VoxelVolume::VoxelVolume()
{
	m_isoLevel = 0.15;
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

		Transform t;
		// TODO - Support raycasting into scaling
		//t.SetScale(m_data.GetScale());
		m_material->RenderInstance(&t);

		m_material->Unbind(window, GetLevel());
	}
}

bool VoxelVolume::Raycast(const Ray& ray, VoxelHitInfo& hit, float maxDistance) 
{
	// Source: https://gist.github.com/yamamushi/5823518
	// Bresenham Line Algorithm 3D

	ivec3 a(
		round(ray.GetOrigin().x),
		round(ray.GetOrigin().y),
		round(ray.GetOrigin().z)
	);
	ivec3 b(
		round(ray.GetOrigin().x + ray.GetDirection().x * maxDistance),
		round(ray.GetOrigin().y + ray.GetDirection().y * maxDistance),
		round(ray.GetOrigin().z + ray.GetDirection().z * maxDistance)
	);
		
	ivec3 point = a;
	ivec3 delta = b - a;
	ivec3 lmn = abs(delta);

	ivec3 inc(
		(delta.x < 0) ? -1 : 1,
		(delta.y < 0) ? -1 : 1,
		(delta.z < 0) ? -1 : 1
	);
	ivec3 delta2(
		lmn.x << 1,
		lmn.y << 1,
		lmn.z << 1
	);

	int err_1;
	int err_2;

#define CHECK_OUTPUT \
	if(point.x >= 0 && point.x < m_data.GetWidth() && point.y >= 0 && point.y < m_data.GetHeight() && point.z >= 0 && point.z < m_data.GetDepth()) \
	{ \
		float value = m_data.Get(point.x, point.y, point.z); \
		if (value >= m_isoLevel) \
		{ \
			hit.coord = point; \
			hit.value = value; \
			return true; \
		} \
	}


	if ((lmn.x >= lmn.y) && (lmn.x >= lmn.z))
	{
		err_1 = delta2.y - 1;
		err_2 = delta2.z - 1;
		for (int i = 0; i < lmn.x; ++i)
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.y += inc.y;
				err_1 -= delta2.x;
			}
			if (err_2 > 0)
			{
				point.z += inc.z;
				err_2 -= delta2.x;
			}
			err_1 += delta2.y;
			err_2 += delta2.z;
			point.x += inc.x;

		}
	}
	else if ((lmn.y >= 1) && (lmn.y >= lmn.z)) 
	{
		err_1 = delta2.x - lmn.y;
		err_2 = delta2.z - lmn.y;
		for (int i = 0; i < lmn.y; ++i) 
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.x += inc.x;
				err_1 -= delta2.y;
			}
			if (err_2 > 0)
			{
				point.z += inc.z;
				err_2 -= delta2.y;
			}
			err_1 += delta2.x;
			err_2 += delta2.z;
			point.y += inc.y;
		}
	}
	else
	{
		err_1 = delta2.y - lmn.z;
		err_2 = delta2.x - lmn.z;
		for (int i = 0; i < lmn.z; ++i)
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.y += inc.y;
				err_1 -= delta2.z;
			}
			if (err_2 > 0)
			{
				point.x += inc.x;
				err_2 -= delta2.z;
			}
			err_1 += delta2.y;
			err_2 += delta2.x;
			point.z += inc.z;
		}
	}

	return false;
}

void VoxelVolume::BuildMesh() 
{
	std::unordered_map<vec3, uint32, vec3_KeyFuncs> vertexIndexLookup;
	vec3 edges[12];

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<uint32> triangles;

	for (uint32 x = 0; x < m_data.GetWidth() - 1; ++x)
		for (uint32 y = 0; y < m_data.GetHeight() - 1; ++y)
			for (uint32 z = 0; z < m_data.GetDepth() - 1; ++z)
			{
				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (m_data.Get(x + 0, y + 0, z + 0) >= m_isoLevel) caseIndex |= 1;
				if (m_data.Get(x + 1, y + 0, z + 0) >= m_isoLevel) caseIndex |= 2;
				if (m_data.Get(x + 1, y + 0, z + 1) >= m_isoLevel) caseIndex |= 4;
				if (m_data.Get(x + 0, y + 0, z + 1) >= m_isoLevel) caseIndex |= 8;
				if (m_data.Get(x + 0, y + 1, z + 0) >= m_isoLevel) caseIndex |= 16;
				if (m_data.Get(x + 1, y + 1, z + 0) >= m_isoLevel) caseIndex |= 32;
				if (m_data.Get(x + 1, y + 1, z + 1) >= m_isoLevel) caseIndex |= 64;
				if (m_data.Get(x + 0, y + 1, z + 1) >= m_isoLevel) caseIndex |= 128;
			

				// Fully inside iso-surface
				if (caseIndex == 0 || caseIndex == 255)
					continue;

				// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) VertexLerp(m_isoLevel, vec3(x + x0,y + y0,z + z0), vec3(x + x1, y + y1, z + z1), m_data.Get(x + x0, y + y0, z + z0), m_data.Get(x + x1, y + y1, z + z1))
				
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