#include "OctreeVolume.h"
#include "Logger.h"
#include "glm.hpp"
#include "DefaultMaterial.h"
#include "InteractionMaterial.h"
#include "Window.h"
#include "Keyboard.h"
#include "MarchingCubes.h"



OctreeVolumeNode::OctreeVolumeNode(OctreeVolumeNode* parent) :
	m_parent(parent),
	m_depth(parent ? parent->m_depth + 1 : 0),
	m_resolution(parent ? parent->m_resolution / 2 : 128)
{
	if (m_parent != nullptr && m_depth == 0)
		LOG_ERROR("OctreeVolumeNode depth overflow");
}

OctreeVolumeNode::OctreeVolumeNode(uint16 resolution) :
	m_parent(nullptr),
	m_depth(0),
	m_resolution(resolution)
{
}

OctreeVolumeNode::~OctreeVolumeNode()
{
}



///
/// Branch
/// 

OctreeVolumeBranch::OctreeVolumeBranch(uint16 resolution) : OctreeVolumeNode(resolution)
{
	if (resolution <= 1)
		LOG_ERROR("Branch node made with size(%i) <= 1", GetResolution());
}

OctreeVolumeBranch::OctreeVolumeBranch(OctreeVolumeNode* parent) : OctreeVolumeNode(parent)
{
	if (GetResolution() <= 1)
		LOG_ERROR("Branch node made with size(%i) <= 1", GetResolution());
}

void OctreeVolumeBranch::Init()
{
	OctreeVolumeBranch* parent = GetParentBranch();
	m_coordinates = parent ? GetParentBranch()->FetchRootCoords(this) : uvec3(0, 0, 0);
}

OctreeVolumeBranch::~OctreeVolumeBranch()
{
	for (OctreeVolumeNode* child : children)
		if (child != nullptr)
			delete child;
}

void OctreeVolumeBranch::Set(uint32 x, uint32 y, uint32 z, float value, std::vector<OctreeVolumeNode*>* additions)
{
	if (x >= GetResolution() || y >= GetResolution() || z >= GetResolution())
	{
		LOG_ERROR("(%i,%i,%i) is outside of volume (resolution:%i)", x, y, z, GetResolution());
		return;
	}

	// Node centre is at halfRes,halfRes,halfRes
	const uint16 halfRes = GetResolution() / 2;


	// Fetch appropriate child node
	bool isFront = (z >= halfRes);
	bool isTop = (y >= halfRes);
	bool isRight = (x >= halfRes);

	uint32 index =
		(isFront ? 4 : 0) +
		(isTop ? 2 : 0) +
		(isRight ? 1 : 0);
	OctreeVolumeNode*& node = children[index];


	// Make sure node exists
	if (node == nullptr)
	{
		// Don't create a node for default value
		if (value == DEFAULT_VALUE)
			return;
		else
		{
			// HalfRes equals the res size of the new node, so make sure to check
			if (halfRes == 1)
				node = new OctreeVolumeLeaf(this);
			else
				node = new OctreeVolumeBranch(this);
			node->Init();

			// Add this new node to additions, if applicable
			if (additions)
				additions->push_back(node);
		}
	}


	// Adjust x,y,z for child node
	node->Set(
		isRight ? x - halfRes : x,
		isTop ? y - halfRes : y,
		isFront ? z - halfRes : z,
		value, additions
	);
	RecalculateStats();
}

float OctreeVolumeBranch::Get(uint32 x, uint32 y, uint32 z) const
{
	if (x >= GetResolution() || y >= GetResolution() || z >= GetResolution())
	{
		LOG_ERROR("(%i,%i,%i) is outside of volume (resolution:%i)", x, y, z, GetResolution());
		return DEFAULT_VALUE;
	}

	// Node centre is at halfRes,halfRes,halfRes
	const uint16 halfRes = GetResolution() / 2;


	// Fetch appropriate child node
	bool isFront = (z >= halfRes);
	bool isTop = (y >= halfRes);
	bool isRight = (x >= halfRes);

	uint32 index =
		(isFront ? 4 : 0) +
		(isTop ? 2 : 0) +
		(isRight ? 1 : 0);
	const OctreeVolumeNode* node = children[index];


	// Node doesn't exist, so is assumed to be default value
	if (node == nullptr)
		return DEFAULT_VALUE;


	// Adjust x,y,z for child node
	return node->Get(
		isRight ? x - halfRes : x,
		isTop ? y - halfRes : y,
		isFront ? z - halfRes : z
	);
}

void OctreeVolumeBranch::RecalculateStats()
{
	// Calculate average
	m_valueAverage = 0.0f;
	for (OctreeVolumeNode* child : children)
		m_valueAverage += child ? child->GetValueAverage() : DEFAULT_VALUE;
	m_valueAverage /= 8.0f;

	// Calulcate std deviation
	float sum = 0.0f;
	for (OctreeVolumeNode* child : children)
		sum += std::pow((child ? child->GetValueAverage() : DEFAULT_VALUE) - m_valueAverage, 2);
	m_valueDeviation = std::sqrt(sum / 8.0f);
}


uvec3 OctreeVolumeBranch::FetchRootCoords(const OctreeVolumeNode* child) const
{
	uvec3 offset;
	const uint32 halfRes = GetResolution() / 2;

	if (child == c000)
		offset = uvec3(0, 0, 0) * halfRes;
	else if (child == c001)
		offset = uvec3(1, 0, 0) * halfRes;
	else if (child == c010)
		offset = uvec3(0, 1, 0) * halfRes;
	else if (child == c011)
		offset = uvec3(1, 1, 0) * halfRes;
	else if (child == c100)
		offset = uvec3(0, 0, 1) * halfRes;
	else if (child == c101)
		offset = uvec3(1, 0, 1) * halfRes;
	else if (child == c110)
		offset = uvec3(0, 1, 1) * halfRes;
	else if (child == c111)
		offset = uvec3(1, 1, 1) * halfRes;

	OctreeVolumeBranch* parentBranch = GetParentBranch();
	return parentBranch ? parentBranch->FetchRootCoords(this) + offset : offset;
}

float OctreeVolumeBranch::FetchBuildIsolevel(const OctreeVolumeNode* source, int32 maxDepth, int32 x, int32 y, int32 z) const
{
	ivec3 offset;
	const int32 res = GetResolution();
	const int32 halfRes = GetResolution() / 2;

	if (source == c000)
		offset = ivec3(0, 0, 0) * halfRes;
	else if (source == c001)
		offset = ivec3(1, 0, 0) * halfRes;
	else if (source == c010)
		offset = ivec3(0, 1, 0) * halfRes;
	else if (source == c011)
		offset = ivec3(1, 1, 0) * halfRes;
	else if (source == c100)
		offset = ivec3(0, 0, 1) * halfRes;
	else if (source == c101)
		offset = ivec3(1, 0, 1) * halfRes;
	else if (source == c110)
		offset = ivec3(0, 1, 1) * halfRes;
	else if (source == c111)
		offset = ivec3(1, 1, 1) * halfRes;
	else
		return UNKNOWN_BUILD_VALUE;


	// Convert x,y,z to this node's local coordinates
	x += offset.x;
	y += offset.y;
	z += offset.z;


	// Desired coord is outside of this node, so traverse up tree
	if (x < 0 || y < 0 || z < 0 || x >= res || y >= res || z >= res)
	{
		auto parent = GetParentBranch();
		if (parent)
			return parent->FetchBuildIsolevel(this, maxDepth, x, y, z);
		else
			return UNKNOWN_BUILD_VALUE; // Most likely reached to of tree (So coord is bad)
	}

	// Desired coord is inside of this node, so traverse down tree
	else
	{
		return FetchBuildIsolevel(maxDepth, x, y, z);
	}
}

float OctreeVolumeBranch::FetchBuildIsolevel(int32 maxDepth, int32 x, int32 y, int32 z) const
{
	if (GetDepth() == maxDepth)
		return GetValueAverage();


	// Fetch appropriate child node
	const uint16 halfRes = GetResolution() / 2;
	bool isFront = (z >= halfRes);
	bool isTop = (y >= halfRes);
	bool isRight = (x >= halfRes);

	uint32 index =
		(isFront ? 4 : 0) +
		(isTop ? 2 : 0) +
		(isRight ? 1 : 0);
	const OctreeVolumeNode* node = children[index];


	// Node doesn't exist, so is assumed to be default value
	if (node == nullptr)
		return DEFAULT_VALUE;


	// Get appropriate value
	const OctreeVolumeBranch* nodeAsBranch = dynamic_cast<const OctreeVolumeBranch*>(node);
	if (nodeAsBranch)
		nodeAsBranch->FetchBuildIsolevel(
			maxDepth,
			isRight ? x - halfRes : x,
			isTop ? y - halfRes : y,
			isFront ? z - halfRes : z
		);

	// Must be at bottom of tree
	else
		return node->Get(
			isRight ? x - halfRes : x,
			isTop ? y - halfRes : y,
			isFront ? z - halfRes : z
		);
}

void OctreeVolumeBranch::ConstructMesh(MeshBuilderMinimal& build, float isoLevel, int32 depthDeltaAcc, int32 depthDeltaDec)
{
	OctreeVolumeBranch* parent = GetParentBranch();
	if (parent == nullptr)
		return;

	// Cache vars
	const vec3 worldCoord = m_coordinates;
	const float resf = (float)GetResolution();
	const int32 maxDepth = GetDepth() - depthDeltaDec;


	// Fetch neighbor values and/or cache them
	std::unordered_map<ivec3, float, ivec3_KeyFuncs> cachedValues;
	auto FetchValue = [this, parent, maxDepth, cachedValues](int32 x, int32 y, int32 z) mutable
	{
		if (x == 0 && y == 0 && z == 0)
			return GetValueAverage();

		const ivec3 key = ivec3(x, y, z);

		// Fetch cached value
		auto it = cachedValues.find(key);
		if (it != cachedValues.end())
			return it->second;

		// Calculate and cache value
		const int32 r = GetResolution();
		float value = parent->FetchBuildIsolevel(this, maxDepth, x*r, y*r, z*r);
		cachedValues[key] = value;
		return value;
	};


	uint8 caseIndex = 0;
	float v000 = FetchValue(0, 0, 0);
	float v001 = FetchValue(0, 0, 1);
	float v010 = FetchValue(0, 1, 0);
	float v011 = FetchValue(0, 1, 1);
	float v100 = FetchValue(1, 0, 0);
	float v101 = FetchValue(1, 0, 1);
	float v110 = FetchValue(1, 1, 0);
	float v111 = FetchValue(1, 1, 1);

	if (v000 >= isoLevel) caseIndex |= 1;
	if (v100 >= isoLevel) caseIndex |= 2;
	if (v101 >= isoLevel) caseIndex |= 4;
	if (v001 >= isoLevel) caseIndex |= 8;
	if (v010 >= isoLevel) caseIndex |= 16;
	if (v110 >= isoLevel) caseIndex |= 32;
	if (v111 >= isoLevel) caseIndex |= 64;
	if (v011 >= isoLevel) caseIndex |= 128;


	// Fully inside iso-surface
	if (caseIndex == 0 || caseIndex == 255)
		return;

	// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(isoLevel, worldCoord + vec3(x0, y0, z0)*resf, worldCoord + vec3(x1, y1, z1)*resf, v ## x0 ## y0 ## z0, v ## x1 ## y1 ## z1);

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

		uint32 a = build.AddVertex(edges[edge0], normal);
		uint32 b = build.AddVertex(edges[edge1], normal);
		uint32 c = build.AddVertex(edges[edge2], normal);

		build.AddTriangle(a, b, c);
	}
}

void OctreeVolumeBranch::ConstructDebugMesh(MeshBuilderMinimal& build, float isoLevel)
{
	if (GetValueAverage() < isoLevel)
		return;


	const vec3 pos = m_coordinates;
	const float res = GetResolution();

	uint32 i000 = build.AddVertex(pos + vec3(0, 0, 0) * res);
	uint32 i001 = build.AddVertex(pos + vec3(0, 0, 1) * res);
	uint32 i010 = build.AddVertex(pos + vec3(0, 1, 0) * res);
	uint32 i011 = build.AddVertex(pos + vec3(0, 1, 1) * res);
	uint32 i100 = build.AddVertex(pos + vec3(1, 0, 0) * res);
	uint32 i101 = build.AddVertex(pos + vec3(1, 0, 1) * res);
	uint32 i110 = build.AddVertex(pos + vec3(1, 1, 0) * res);
	uint32 i111 = build.AddVertex(pos + vec3(1, 1, 1) * res);


	build.AddTriangle(i000, i001, i011);
	build.AddTriangle(i000, i011, i010);
	build.AddTriangle(i100, i101, i111);
	build.AddTriangle(i100, i111, i110);

	build.AddTriangle(i000, i001, i101);
	build.AddTriangle(i000, i101, i100);
	build.AddTriangle(i010, i011, i111);
	build.AddTriangle(i010, i111, i110);

	build.AddTriangle(i000, i010, i110);
	build.AddTriangle(i000, i110, i100);
	build.AddTriangle(i001, i011, i111);
	build.AddTriangle(i001, i111, i101);

	for (OctreeVolumeNode* child : children)
		if (child)
			child->ConstructDebugMesh(build, isoLevel);
}



///
/// Leaf
///

OctreeVolumeLeaf::OctreeVolumeLeaf(OctreeVolumeNode* parent) : OctreeVolumeNode(parent), m_value(DEFAULT_VALUE)
{
}
OctreeVolumeLeaf::~OctreeVolumeLeaf()
{
	if (m_meshData != nullptr)
		delete m_meshData;
}

void OctreeVolumeLeaf::Init()
{
	m_coordinates = GetParentBranch()->FetchRootCoords(this);
}

void OctreeVolumeLeaf::Set(uint32 x, uint32 y, uint32 z, float value, std::vector<OctreeVolumeNode*>* additions) 
{ 
	m_value = value; 
	if (m_meshData)
		m_meshData->isStale = true;
}

void OctreeVolumeLeaf::ConstructMesh(MeshBuilderMinimal& build, float isoLevel, int32 depthDeltaAcc, int32 depthDeltaDec)
{
	OctreeVolumeBranch* parent = GetParentBranch();
	if (parent == nullptr)
		return;

	// Only rebuild mesh data if something has changed
	if (m_meshData == nullptr || m_meshData->isStale)
	{
		// Create place to store mesh data for this node
		if (m_meshData == nullptr)
			m_meshData = new VoxelPartialMeshData;
		m_meshData->Clear();
		m_meshData->isStale = false;


		// Cache vars
		const vec3 worldCoord = m_coordinates;
		const float resf = (float)GetResolution();
		const int32 maxDepth = GetDepth() - depthDeltaDec;


		// Fetch neighbor values and/or cache them
		std::unordered_map<ivec3, float, ivec3_KeyFuncs> cachedValues;
		auto FetchValue = [this, parent, maxDepth, cachedValues](int32 x, int32 y, int32 z) mutable
		{
			if (x == 0 && y == 0 && z == 0)
				return GetValueAverage();

			const ivec3 key = ivec3(x, y, z);

			// Fetch cached value
			auto it = cachedValues.find(key);
			if (it != cachedValues.end())
				return it->second;

			// Calculate and cache value
			float value = parent->FetchBuildIsolevel(this, maxDepth, x, y, z);
			cachedValues[key] = value;
			return value;
		};


		uint8 caseIndex = 0;
		float v000 = FetchValue(0, 0, 0);
		float v001 = FetchValue(0, 0, 1);
		float v010 = FetchValue(0, 1, 0);
		float v011 = FetchValue(0, 1, 1);
		float v100 = FetchValue(1, 0, 0);
		float v101 = FetchValue(1, 0, 1);
		float v110 = FetchValue(1, 1, 0);
		float v111 = FetchValue(1, 1, 1);

		if (v000 >= isoLevel) caseIndex |= 1;
		if (v100 >= isoLevel) caseIndex |= 2;
		if (v101 >= isoLevel) caseIndex |= 4;
		if (v001 >= isoLevel) caseIndex |= 8;
		if (v010 >= isoLevel) caseIndex |= 16;
		if (v110 >= isoLevel) caseIndex |= 32;
		if (v111 >= isoLevel) caseIndex |= 64;
		if (v011 >= isoLevel) caseIndex |= 128;


		// Fully inside iso-surface
		if (caseIndex == 0 || caseIndex == 255)
			return;


		// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(isoLevel, worldCoord + vec3(x0, y0, z0)*resf, worldCoord + vec3(x1, y1, z1)*resf, v ## x0 ## y0 ## z0, v ## x1 ## y1 ## z1);

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
			m_meshData->AddTriangle(edges[edge0], edges[edge1], edges[edge2], normal);
		}
	}


	// Add triangle data to the mesh
	for (const auto& tri : m_meshData->triangles)
	{
		uint32 a = build.AddVertex(tri.a, tri.weightedNormal);
		uint32 b = build.AddVertex(tri.b, tri.weightedNormal);
		uint32 c = build.AddVertex(tri.c, tri.weightedNormal);

		build.AddTriangle(a, b, c);
	}
}

void OctreeVolumeLeaf::ConstructDebugMesh(MeshBuilderMinimal& build, float isoLevel)
{
	if (m_value < isoLevel)
		return;

	const vec3 pos = m_coordinates;
	const float res = GetResolution();

	uint32 i000 = build.AddVertex(pos + vec3(0, 0, 0) * res);
	uint32 i001 = build.AddVertex(pos + vec3(0, 0, 1) * res);
	uint32 i010 = build.AddVertex(pos + vec3(0, 1, 0) * res);
	uint32 i011 = build.AddVertex(pos + vec3(0, 1, 1) * res);
	uint32 i100 = build.AddVertex(pos + vec3(1, 0, 0) * res);
	uint32 i101 = build.AddVertex(pos + vec3(1, 0, 1) * res);
	uint32 i110 = build.AddVertex(pos + vec3(1, 1, 0) * res);
	uint32 i111 = build.AddVertex(pos + vec3(1, 1, 1) * res);


	build.AddTriangle(i000, i001, i011);
	build.AddTriangle(i000, i011, i010);
	build.AddTriangle(i100, i101, i111);
	build.AddTriangle(i100, i111, i110);

	build.AddTriangle(i000, i001, i101);
	build.AddTriangle(i000, i101, i100);
	build.AddTriangle(i010, i011, i111);
	build.AddTriangle(i010, i111, i110);

	build.AddTriangle(i000, i010, i110);
	build.AddTriangle(i000, i110, i100);
	build.AddTriangle(i001, i011, i111);
	build.AddTriangle(i001, i111, i101);
}



///
/// Octree Object
///

OctreeVolume::OctreeVolume()
{
	m_isoLevel = 0.15f;
}

OctreeVolume::~OctreeVolume()
{
	if (m_material != nullptr)
		delete m_material;
	if (m_wireMaterial != nullptr)
		delete m_wireMaterial;
	if (m_root != nullptr)
		delete m_root;

	if (m_mesh != nullptr)
		delete m_mesh;
	if (TEST_MESH != nullptr)
		delete TEST_MESH;
}


///
/// Object functions
///

void OctreeVolume::Begin()
{
	m_material = new DefaultMaterial;
	m_wireMaterial = new InteractionMaterial;

	m_mesh = new Mesh;
	m_mesh->MarkDynamic();

	TEST_MESH = new Mesh;
	TEST_MESH->MarkDynamic();

	BuildMesh();
	//BuildTreeMesh();

	TEST_REBUILD = false;
}

void OctreeVolume::Update(const float & deltaTime)
{
	if (TEST_REBUILD)
	{
		BuildMesh();
		//BuildTreeMesh();
		TEST_REBUILD = false;
	}
}

void OctreeVolume::Draw(const Window * window, const float & deltaTime)
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
			//m_wireMaterial->PrepareMesh(TEST_MESH);
			m_wireMaterial->RenderInstance(&t);
			m_wireMaterial->Unbind(window, GetLevel());
		}
	}
}


///
/// Object functions
///

void OctreeVolume::Init(const uvec3 & resolution, const vec3 & scale)
{
	// Use even square resolution
	const uint32 max = glm::max(resolution.x, glm::max(resolution.y, resolution.z));
	const uint32 res = glm::max((uint32)pow(2, ceil(log(max) / log(2))), 2U);


	LOG("OctreeVolume uses power of 2 uniform resolution");
	LOG("(%i,%i,%i) converted to %ix", resolution.x, resolution.y, resolution.z, res);

	m_resolution = uvec3(res, res, res);
	m_scale = scale;
	m_root = new OctreeVolumeBranch(res);
}

void OctreeVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	std::vector<OctreeVolumeNode*> newNodes;
	m_root->Set(x, y, z, value, &newNodes);

	// Add all nodes in bottom layer
	for (OctreeVolumeNode* node : newNodes)
		if (node->GetResolution() == 1)
			testLayer.push_back(node);

	TEST_REBUILD = true;
}

float OctreeVolume::Get(uint32 x, uint32 y, uint32 z)
{
	return m_root->Get(x, y, z);
}

void OctreeVolume::BuildMesh() 
{
	MeshBuilderMinimal builder;
	builder.MarkDynamic();

	for (OctreeVolumeNode* node : testLayer)
		node->ConstructMesh(builder, m_isoLevel, 0, 0);

	builder.BuildMesh(m_mesh);
}

void OctreeVolume::BuildTreeMesh()
{
	MeshBuilderMinimal builder;
	builder.MarkDynamic();

	for (OctreeVolumeNode* node : testLayer)
		node->ConstructDebugMesh(builder, m_isoLevel);

	builder.BuildMesh(TEST_MESH);
}