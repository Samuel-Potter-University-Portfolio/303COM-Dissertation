#include "OctreeRepVolume.h"
#include "Logger.h"

#include "DefaultMaterial.h"
#include "InteractionMaterial.h"

#include "MarchingCubes.h"
#include "MeshBuilder.h"

#include "Window.h"
#include "Keyboard.h"



///
/// Node
///
OctRepNode::OctRepNode(uint16 resolution) :
	m_parent(nullptr),
	m_resolution(resolution),
	m_offset(0, 0, 0), 
	m_depth(0)
{
}
OctRepNode::OctRepNode(OctRepNode* parent, const uvec3& offset) :
	m_parent(parent),
	m_resolution(parent->GetChildResolution()),
	m_offset(offset),
	m_depth(parent->m_depth + 1)
{
}
OctRepNode::~OctRepNode() 
{
	for (auto child : m_children)
		if(child != nullptr)
			delete child;
}

void OctRepNode::Push(const uint32& x, const uint32& y, const uint32& z, const float& value, OctRepNotifyPacket& outPacket)
{
	// Update value at leaf
	if (IsLeaf())
	{
		if (m_values[GetIndex(x, y, z)] != value)
		{
			m_values[GetIndex(x, y, z)] = value;
			RecalculateStats();
			outPacket.updatedNodes.emplace_back(this);
		}
		return;
	}

	// One of the corners, so save the value
	if ((x == 0 || x == GetResolution() - 1) &&
		(y == 0 || y == GetResolution() - 1) &&
		(z == 0 || z == GetResolution() - 1)
		) 
	{
		m_values[GetIndex(x == 0 ? 0 : 1, y == 0 ? 0 : 1, z == 0 ? 0 : 1)] = value;
	}


	const uint16 childRes = GetChildResolution();

	// Check each child to see if this is in range of interest
	for (uint32 i = 0; i < 8; ++i) 
	{
		// Work out child coords
		const uint32 cx = i % 2;
		const uint32 cy = (i / 2) % 2;
		const uint32 cz = (i / 2) / 2;

		const int32 ox = cx * (childRes - 1);
		const int32 oy = cy * (childRes - 1);
		const int32 oz = cz * (childRes - 1);

		// Check child cares about this value
		// Children have range of 0-childRes (This range will be offset by cX/Y/Z)
		if (x >= ox && x < childRes + ox && 
			y >= oy && y < childRes + oy && 
			z >= oz && z < childRes + oz)
		{
			auto& child = m_children[i];

			// Create new child, if not default value
			if (child == nullptr && value != DEFAULT_VALUE)
			{
				child = new OctRepNode(this, m_offset + uvec3(ox, oy, oz));
				outPacket.newNodes.emplace_back(child);
			}

			// Update value in child
			if (child != nullptr)
			{
				// Transform coords into child's local coords
				uint32 nx = cx == 0 ? x : x - (childRes - 1);
				uint32 ny = cy == 0 ? y : y - (childRes - 1);
				uint32 nz = cz == 0 ? z : z - (childRes - 1);

				child->Push(nx, ny, nz, value, outPacket);
			}
		}
	}

	// Check children are not default values
	for (auto& child : m_children)
	{
		if (child != nullptr && child->IsDefaultValues())
		{
			outPacket.deletedNodes.emplace_back(child);
			delete child;
			child = nullptr;
		}
	}


	// Update stats
	if (!IsDefaultValues())
	{
		float oldAverage = m_average;
		float oldDeviation = m_stdDeviation;
		RecalculateStats();

		// Has this node been updated
		if (oldAverage != m_average || oldDeviation != m_stdDeviation)
			outPacket.updatedNodes.emplace_back(this);
	}
}

void OctRepNode::RecalculateStats()
{
	// Calculate average
	m_average = 0.0f;
	for (int i = 0; i < 8; ++i)
		m_average += m_values[i];
	m_average /= 8.0f;

	// Calulcate std deviation
	float sum = 0.0f;
	for (int i = 0; i < 8; ++i)
		sum += std::pow(m_values[i] - m_average, 2);
	m_stdDeviation = std::sqrt(sum / 8.0f);
}

uint32 OctRepNode::GetChildResolution() const 
{
	// Where res(i) = 1 + 2^(i+1)
	return m_resolution > 3 ? (m_resolution - 1) / 2 + 1 : 2;
}

bool OctRepNode::IsDefaultValues() const
{
	if (IsLeaf())
	{
		for (const float& value : m_values)
			if (value != DEFAULT_VALUE)
				return false;
		return true;
	}
	else
	{
		for (const auto& child : m_children)
			if (child != nullptr)
				return false;
		return true;
	}
}

void OctRepNode::GeneratePartialMesh(const float& isoLevel, VoxelPartialMeshData* target)
{
	uint8 caseIndex = 0;
	if (m_values[GetIndex(0, 0, 0)] >= isoLevel) caseIndex |= 1;
	if (m_values[GetIndex(1, 0, 0)] >= isoLevel) caseIndex |= 2;
	if (m_values[GetIndex(1, 0, 1)] >= isoLevel) caseIndex |= 4;
	if (m_values[GetIndex(0, 0, 1)] >= isoLevel) caseIndex |= 8;
	if (m_values[GetIndex(0, 1, 0)] >= isoLevel) caseIndex |= 16;
	if (m_values[GetIndex(1, 1, 0)] >= isoLevel) caseIndex |= 32;
	if (m_values[GetIndex(1, 1, 1)] >= isoLevel) caseIndex |= 64;
	if (m_values[GetIndex(0, 1, 1)] >= isoLevel) caseIndex |= 128;

	// Fully inside iso-surface
	if (caseIndex == 0 || caseIndex == 255)
		return;


	// Cache vars
	const vec3 worldCoord = m_offset;
	const float resf = (float)GetResolution() - 1.0f;


	// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(isoLevel, worldCoord + vec3(x0, y0, z0)*resf, worldCoord + vec3(x1, y1, z1)*resf, m_values[GetIndex(x0,y0,z0)], m_values[GetIndex(x1,y1,z1)]);
	vec3 edges[12];

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
		int8 edge0 = *(caseEdges++);
		int8 edge1 = *(caseEdges++);
		int8 edge2 = *(caseEdges++);

		vec3 normal = glm::cross(edges[edge1] - edges[edge0], edges[edge2] - edges[edge0]);
		target->AddTriangle(edges[edge0], edges[edge1], edges[edge2], normal);
	}
}



///
/// Octree object
///

OctreeRepVolume::OctreeRepVolume()
{
	m_isoLevel = 0.15f;
}
OctreeRepVolume::~OctreeRepVolume()
{
	if (m_material != nullptr)
		delete m_material;
	if (m_wireMaterial != nullptr)
		delete m_wireMaterial;
	if (m_mesh != nullptr)
		delete m_mesh;

	if (m_octree != nullptr)
		delete m_octree;
}

//
// Object functions
//

void OctreeRepVolume::Begin() 
{
	m_material = new DefaultMaterial;
	m_wireMaterial = new InteractionMaterial;

	m_mesh = new Mesh;
	m_mesh->MarkDynamic();
}

void OctreeRepVolume::Update(const float & deltaTime)
{
	if (TEST_REBUILD)
	{
		BuildMesh();
		TEST_REBUILD = false;
	}
}

void OctreeRepVolume::Draw(const Window * window, const float & deltaTime)
{
	static bool drawWireFrame = false;
	static bool drawMainObject = true;
	const Keyboard* keyboard = window->GetKeyboard();

	if (keyboard->IsKeyPressed(Keyboard::Key::KV_T))
		drawWireFrame = !drawWireFrame;
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_R))
		drawMainObject = !drawMainObject;


	if (m_mesh != nullptr)
	{
		Transform t;

		if (drawMainObject)
		{
			m_material->Bind(window, GetLevel());
			m_material->PrepareMesh(m_mesh);
			m_material->RenderInstance(&t);
			m_material->Unbind(window, GetLevel());
		}

		if (drawWireFrame)
		{
			m_wireMaterial->Bind(window, GetLevel());
			m_wireMaterial->PrepareMesh(m_mesh);
			m_wireMaterial->RenderInstance(&t);
			m_wireMaterial->Unbind(window, GetLevel());
		}
	}
}


//
// Volume functions
//

void OctreeRepVolume::Init(const uvec3 & resolution, const vec3 & scale)
{
	const uint32 max = glm::max(resolution.x, glm::max(resolution.y, resolution.z));

	// Octree res scales by f(n) = 1 + 2^(n+1)
	// As each leaf node contains potential overlapping values
	// Meaning the parent of the leafs has a res of 5
	uint32 res = 0;
	for (uint32 i = 0; res < max; ++i)
		res = 1 + pow(2, i + 1);

	LOG("OctreeRepVolume using normal_res: (%i,%i,%i) octree_res: (%i,%i,%i)", resolution.x, resolution.y, resolution.z, res, res, res);

	m_resolution = resolution;
	m_scale = scale;
	m_data.clear();
	m_data.resize(resolution.x * resolution.y * resolution.z);
	m_octree = new OctRepNode(res);
}

void OctreeRepVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	m_data[GetIndex(x, y, z)] = value;

	OctRepNotifyPacket changes;
	m_octree->Push(x, y, z, value, changes);


	// Remove deleted nodes
	for (OctRepNode* node : changes.deletedNodes)
	{
		auto it = m_nodeLevel.find(node);
		if (it != m_nodeLevel.end())
			m_nodeLevel.erase(it);
	}

	// Add leaf nodes to list
	for (OctRepNode* node : changes.newNodes)
		if (node->GetResolution() == 17)
			(m_nodeLevel[node] = VoxelPartialMeshData()).isStale = true;


	// Update leaf nodes to list
	for (OctRepNode* node : changes.updatedNodes)
	{
		auto it = m_nodeLevel.find(node);
		if (it != m_nodeLevel.end())
			it->second.isStale = true;
	}


	TEST_REBUILD = true;
}

float OctreeRepVolume::Get(uint32 x, uint32 y, uint32 z)
{
	return m_data[GetIndex(x, y, z)];
}

void OctreeRepVolume::BuildMesh() 
{
	// Regen partial meshes and add them together to make main mesh
	MeshBuilderMinimal builder;
	builder.MarkDynamic();

	for (auto& pair : m_nodeLevel)
	{
		// Rebuild mesh data
		if (pair.second.isStale)
		{
			pair.second.Clear();
			pair.first->GeneratePartialMesh(m_isoLevel, &pair.second);
			pair.second.isStale = false;
		}

		// Add triangles to mesh
		for (const auto& tri : pair.second.triangles)
		{
			const uint32 a = builder.AddVertex(tri.a, tri.weightedNormal);
			const uint32 b = builder.AddVertex(tri.b, tri.weightedNormal);
			const uint32 c = builder.AddVertex(tri.c, tri.weightedNormal);
			builder.AddTriangle(a, b, c);
		}
	}

	builder.BuildMesh(m_mesh);
}