#include "LayeredVolume.h"
#include "Window.h"

#include "DefaultMaterial.h"
#include "InteractionMaterial.h"

#include "Logger.h"
#include "MarchingCubes.h"


///
/// Node
///

void OctreeLayerNode::Push(const uint32& corner, const LayeredVolume* volume, const float& value)
{
	m_values[corner] = value;

	// Update case index
	const float isoLevel = volume->GetIsoLevel();

	// Set bit to 1
	if (value >= isoLevel)
	{
		if		(corner == CHILD_OFFSET_BACK_BOTTOM_LEFT)	m_caseIndex |= 1;
		else if (corner == CHILD_OFFSET_BACK_BOTTOM_RIGHT)	m_caseIndex |= 2;
		else if (corner == CHILD_OFFSET_FRONT_BOTTOM_RIGHT) m_caseIndex |= 4;
		else if (corner == CHILD_OFFSET_FRONT_BOTTOM_LEFT)	m_caseIndex |= 8;
		else if (corner == CHILD_OFFSET_BACK_TOP_LEFT)		m_caseIndex |= 16;
		else if (corner == CHILD_OFFSET_BACK_TOP_RIGHT)		m_caseIndex |= 32;
		else if (corner == CHILD_OFFSET_FRONT_TOP_RIGHT)	m_caseIndex |= 64;
		else if (corner == CHILD_OFFSET_FRONT_TOP_LEFT)		m_caseIndex |= 128;
		else { LOG_ERROR("Invalid corner used in push"); }
	}
	// Set bit to 0
	else
	{
		if		(corner == CHILD_OFFSET_BACK_BOTTOM_LEFT)	m_caseIndex &= ~1;
		else if (corner == CHILD_OFFSET_BACK_BOTTOM_RIGHT)	m_caseIndex &= ~2;
		else if (corner == CHILD_OFFSET_FRONT_BOTTOM_RIGHT) m_caseIndex &= ~4;
		else if (corner == CHILD_OFFSET_FRONT_BOTTOM_LEFT)	m_caseIndex &= ~8;
		else if (corner == CHILD_OFFSET_BACK_TOP_LEFT)		m_caseIndex &= ~16;
		else if (corner == CHILD_OFFSET_BACK_TOP_RIGHT)		m_caseIndex &= ~32;
		else if (corner == CHILD_OFFSET_FRONT_TOP_RIGHT)	m_caseIndex &= ~64;
		else if (corner == CHILD_OFFSET_FRONT_TOP_LEFT)		m_caseIndex &= ~128;
		else { LOG_ERROR("Invalid corner used in push"); }
	}

	// TODO - Calculate edges
}

void OctreeLayerNode::BuildMesh(const float& isoLevel, MeshBuilderMinimal& builder)
{
	if (m_caseIndex == 255)
		return;

	const uint32 stride = m_layer->GetStride();
	const uvec3 layerCoords = m_layer->GetLocalCoords(m_id);
	const float stridef = stride;
	const vec3 coordsf = layerCoords;
	

	// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(isoLevel, (coordsf + vec3(x0,y0,z0)) * stridef, (coordsf + vec3(x1,y1,z1)) * stridef, m_values[GetIndex(x0, y0, z0)], m_values[GetIndex(x1, y1, z1)])
	vec3 edges[12];

	if (MC::CaseRequiredEdges[m_caseIndex] & 1)
		edges[0] = VERT_LERP(0, 0, 0, 1, 0, 0);
	if (MC::CaseRequiredEdges[m_caseIndex] & 2)
		edges[1] = VERT_LERP(1, 0, 0, 1, 0, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 4)
		edges[2] = VERT_LERP(0, 0, 1, 1, 0, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 8)
		edges[3] = VERT_LERP(0, 0, 0, 0, 0, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 16)
		edges[4] = VERT_LERP(0, 1, 0, 1, 1, 0);
	if (MC::CaseRequiredEdges[m_caseIndex] & 32)
		edges[5] = VERT_LERP(1, 1, 0, 1, 1, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 64)
		edges[6] = VERT_LERP(0, 1, 1, 1, 1, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 128)
		edges[7] = VERT_LERP(0, 1, 0, 0, 1, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 256)
		edges[8] = VERT_LERP(0, 0, 0, 0, 1, 0);
	if (MC::CaseRequiredEdges[m_caseIndex] & 512)
		edges[9] = VERT_LERP(1, 0, 0, 1, 1, 0);
	if (MC::CaseRequiredEdges[m_caseIndex] & 1024)
		edges[10] = VERT_LERP(1, 0, 1, 1, 1, 1);
	if (MC::CaseRequiredEdges[m_caseIndex] & 2048)
		edges[11] = VERT_LERP(0, 0, 1, 0, 1, 1);

	

	// Add triangles for this case
	int8* caseEdges = MC::Cases[m_caseIndex];
	while (*caseEdges != -1)
	{
		int8 edge0 = *(caseEdges++);
		int8 edge1 = *(caseEdges++);
		int8 edge2 = *(caseEdges++);

		vec3 normal = glm::cross(edges[edge1] - edges[edge0], edges[edge2] - edges[edge0]);
		
		const uint32 a = builder.AddVertex(edges[edge0], normal);
		const uint32 b = builder.AddVertex(edges[edge1], normal);
		const uint32 c = builder.AddVertex(edges[edge2], normal);
		builder.AddTriangle(a, b, c);
	}
}


///
/// Layer
///

OctreeLayer::OctreeLayer(LayeredVolume* volume, const uint32& depth, const uint32& height, const uint32& nodeRes) :
	m_volume(volume),
	m_depth(depth), m_nodeResolution(nodeRes), m_layerResolution((volume->GetOctreeResolution() - 1) / GetStride() + 1),
	m_startIndex(depth == 0 ? 0 : (pow(8, depth) - 1) / 7) // Total nodes for a given height is (8^h - 1)/7
{
}

OctreeLayer::~OctreeLayer() 
{
	// Delete nodes
	for (auto pair : m_nodes)
		delete pair.second;
}

bool OctreeLayer::HandlePush(const uint32& x, const uint32& y, const uint32& z, const float& value) 
{
	// Check this resolution cares about this value
	const uint32 stride = GetStride();

	// Attempt to push the value onto nodes in this layer
	if (x % stride != 0 || y % stride != 0 || z % stride != 0)
		return false;

	const uvec3 local = uvec3(x, y, z) / stride;

	PushValueOntoNode(local, ivec3(0, 0, 0), value);
	PushValueOntoNode(local, ivec3(0, 0, -1), value);
	PushValueOntoNode(local, ivec3(0, -1, 0), value);
	PushValueOntoNode(local, ivec3(0, -1, -1), value);
	PushValueOntoNode(local, ivec3(-1, 0, 0), value);
	PushValueOntoNode(local, ivec3(-1, 0, -1), value);
	PushValueOntoNode(local, ivec3(-1, -1, 0), value);
	PushValueOntoNode(local, ivec3(-1, -1, -1), value);

	// Push on
}

void OctreeLayer::PushValueOntoNode(const uvec3& localCoords, const ivec3& offset, const float& value)
{
	uvec3 nodeCoords = uvec3(localCoords.x + offset.x, localCoords.y + offset.y, localCoords.z + offset.z);
	const uint32 id = GetID(nodeCoords.x, nodeCoords.y, nodeCoords.z);

	// If id is outside of node range of this layer offset has overflowed
	if (id < m_startIndex || id >= m_startIndex + m_layerResolution*m_layerResolution*m_layerResolution)
		return;


	// Find node
	auto it = m_nodes.find(id);
	OctreeLayerNode* node;

	if (it == m_nodes.end()) // No node of this id exists
	{
		// Create node, if it's worth creating it
		if (value >= m_volume->GetIsoLevel())
		{
			node = new OctreeLayerNode(id, this);
			m_nodes[id] = node;
			
			// Init all corner node for this child
			const uint32 stride = GetStride();
			node->Push(CHILD_OFFSET_BACK_BOTTOM_LEFT,	m_volume, m_volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 0) * stride));
			node->Push(CHILD_OFFSET_BACK_BOTTOM_RIGHT,	m_volume, m_volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 0) * stride));
			node->Push(CHILD_OFFSET_BACK_TOP_LEFT,		m_volume, m_volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 0) * stride));
			node->Push(CHILD_OFFSET_BACK_TOP_RIGHT,		m_volume, m_volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 0) * stride));
			node->Push(CHILD_OFFSET_FRONT_BOTTOM_LEFT,	m_volume, m_volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 1) * stride));
			node->Push(CHILD_OFFSET_FRONT_BOTTOM_RIGHT, m_volume, m_volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 1) * stride));
			node->Push(CHILD_OFFSET_FRONT_TOP_LEFT,		m_volume, m_volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 1) * stride));
			node->Push(CHILD_OFFSET_FRONT_TOP_RIGHT,	m_volume, m_volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 1) * stride));
		}
		else
			return;

	}
	else
	{
		node = it->second;

		const uint32 corner = -offset.x + 2 * (-offset.y + 2 * -offset.z); // The id of the corner of this value
		node->Push(corner, m_volume, value);
	}

	// Node is now fully out the surface, so remove it
	if (node->FlaggedForDeletion())
	{
		m_nodes.erase(id);
		delete node;
	}
}

void OctreeLayer::BuildMesh(MeshBuilderMinimal& builder)
{
	for (auto node : m_nodes)
		node.second->BuildMesh(m_volume->GetIsoLevel(), builder);
}

bool OctreeLayer::AttemptNodeFetch(const uint32& id, OctreeLayerNode*& outNode) const
{
	// Node in previous layer
	if (id < m_startIndex)
	{
		if (previousLayer)
			return previousLayer->AttemptNodeFetch(id, outNode);
		else
			return false;
	}
	// Node in next layer
	else if (id >= m_startIndex + m_layerResolution*m_layerResolution*m_layerResolution)
	{
		if (nextLayer)
			return nextLayer->AttemptNodeFetch(id, outNode);
		else
			return false;
	}
	// Node must be in this layer
	else
	{
		auto it = m_nodes.find(id);
		if (it == m_nodes.end())
			return false;
		else
		{
			outNode = it->second;
			return true;
		}
	}
}


///
/// Object
///

LayeredVolume::LayeredVolume()
{
	m_isoLevel = 0.15f;
}
LayeredVolume::~LayeredVolume()
{
	if (m_material != nullptr)
		delete m_material;
	if (m_wireMaterial != nullptr)
		delete m_wireMaterial;

	if (m_debugMaterial != nullptr)
		delete m_debugMaterial;

	for (OctreeLayer* layer : m_layers)
		delete layer;
}

//
// Object functions
//

void LayeredVolume::Begin()
{
	m_material = new DefaultMaterial;
	m_wireMaterial = new InteractionMaterial(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	m_debugMaterial = new InteractionMaterial(vec4(0.0f, 0.0f, 0.0f, 1.0f));

	TEST_MESH = new Mesh;
	TEST_MESH->MarkDynamic();
}

void LayeredVolume::Update(const float & deltaTime)
{
	if (TEST_REBUILD)
	{
		//BuildMesh();
		MeshBuilderMinimal builder;
		m_layers[0]->BuildMesh(builder);
		//m_layers[m_layers.size() - 1].BuildMesh(builder);
		builder.BuildMesh(TEST_MESH);

		LOG("Build");
		TEST_REBUILD = false;
	}
}

void LayeredVolume::Draw(const Window * window, const float & deltaTime)
{
	static bool drawWireFrame = false;
	static bool drawMainObject = true;
	const Keyboard* keyboard = window->GetKeyboard();

	if (keyboard->IsKeyPressed(Keyboard::Key::KV_T))
		drawWireFrame = !drawWireFrame;
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_R))
		drawMainObject = !drawMainObject;


	if (TEST_MESH != nullptr)
	{
		Transform t;

		if (drawMainObject)
		{
			m_material->Bind(window, GetLevel());
			m_material->PrepareMesh(TEST_MESH);
			m_material->RenderInstance(&t);
			m_material->Unbind(window, GetLevel());
		}

		if (drawWireFrame)
		{
			m_wireMaterial->Bind(window, GetLevel());
			m_wireMaterial->PrepareMesh(TEST_MESH);
			m_wireMaterial->RenderInstance(&t);
			m_wireMaterial->Unbind(window, GetLevel());
		}
	}
	/*
	if (keyboard->IsKeyDown(Keyboard::Key::KV_F) && m_debugMesh != nullptr && m_debugMaterial != nullptr)
	{
		Transform t;
		m_debugMaterial->Bind(window, GetLevel());
		m_debugMaterial->PrepareMesh(m_debugMesh);
		m_debugMaterial->RenderInstance(&t);
		m_debugMaterial->Unbind(window, GetLevel());
	}

	// Completely rebuild mesh
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_N))
	{
		for (auto& pair : m_nodeLevel)
			pair.second.isStale = true;
		for (auto& pair : m_nodedebugLevel)
			pair.second.isStale = true;

		BuildMesh();
		LOG("Done");
	}
	*/
}


//
// Volume functions
//

void LayeredVolume::Init(const uvec3 & resolution, const vec3 & scale)
{
	const uint32 max = glm::max(resolution.x, glm::max(resolution.y, resolution.z));

	// Octree res scales by f(n) = 1 + 2^(n+1)
	// As each leaf node contains potential overlapping values
	// Meaning the parent of the leafs has a res of 5
	uint32 res = 0;
	for (uint32 i = 0; res < max; ++i)
		res = 1 + pow(2U, i + 1);
	m_octreeRes = res;

	LOG("LayeredVolume using normal_res: (%i,%i,%i) octree_res: (%i,%i,%i)", resolution.x, resolution.y, resolution.z, res, res, res);

	m_resolution = resolution;
	m_scale = scale;
	m_data.clear();
	m_data.resize(resolution.x * resolution.y * resolution.z);


	// Allocate number of layers
	const uint32 maxDepth = (uint32)ceil(log2(res - 1)) + 1;
	const uint32 layerCount = 4; // Check layer count is not greater than maxDepth
	m_layers.clear();
	OctreeLayer* previousLayer = nullptr;

	// Create layers
	for (uint32 i = maxDepth - layerCount; i < maxDepth; ++i)
	{
		// Make sure layers are created from highest height to lowest height
		const uint32 j = maxDepth - i;

		const uint32 layerHeight = j;
		const uint32 layerDepth = maxDepth - layerHeight;
		const uint32 nodeRes = ((res - 1) / pow(2, layerDepth)) + 1;

		OctreeLayer* layer = new OctreeLayer(this, layerDepth, layerHeight, nodeRes);

		// Connect layers
		layer-> previousLayer = previousLayer;
		if (previousLayer != nullptr)
			previousLayer->nextLayer = layer;

		m_layers.push_back(layer);
		previousLayer = layer;
	}

}

void LayeredVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	m_data[GetIndex(x, y, z)] = value;

	// Notify any layers of any changes
	for (OctreeLayer* layer : m_layers)
		TEST_REBUILD |= layer->HandlePush(x, y, z, value);
}

float LayeredVolume::Get(uint32 x, uint32 y, uint32 z)
{
	if (x >= m_resolution.x || y >= m_resolution.y || z >= m_resolution.z)
		return UNKNOWN_BUILD_VALUE;

	return m_data[GetIndex(x, y, z)];
}

float LayeredVolume::Get(uint32 x, uint32 y, uint32 z) const
{
	if (x >= m_resolution.x || y >= m_resolution.y || z >= m_resolution.z)
		return UNKNOWN_BUILD_VALUE;

	return m_data[GetIndex(x, y, z)];
}