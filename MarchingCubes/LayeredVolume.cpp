#include "LayeredVolume.h"
#include "Window.h"

#include "DefaultMaterial.h"
#include "InteractionMaterial.h"

#include "Logger.h"
#include "MarchingCubes.h"

#include <unordered_set>


///
/// Node
///

OctreeLayerNode::OctreeLayerNode(const uint32& id, OctreeLayer* layer, const bool& initaliseValues) :
	m_id(id), m_values({ DEFAULT_VALUE }), m_layer(layer) 
{
	if (initaliseValues)
	{
		const LayeredVolume* volume = layer->GetVolume();
		const uint32 stride = layer->GetStride();
		const uvec3 nodeCoords = layer->GetLocalCoords(id);
		const float isoLevel = volume->GetIsoLevel();

		m_values[CHILD_OFFSET_BK_BOT_L] = volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 0) * stride);
		m_values[CHILD_OFFSET_BK_BOT_R] = volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 0) * stride);
		m_values[CHILD_OFFSET_BK_TOP_L] = volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 0) * stride);
		m_values[CHILD_OFFSET_BK_TOP_R] = volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 0) * stride);
		m_values[CHILD_OFFSET_FR_BOT_L] = volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 1) * stride);
		m_values[CHILD_OFFSET_FR_BOT_R] = volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 0) * stride, (nodeCoords.z + 1) * stride);
		m_values[CHILD_OFFSET_FR_TOP_L] = volume->Get((nodeCoords.x + 0) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 1) * stride);
		m_values[CHILD_OFFSET_FR_TOP_R] = volume->Get((nodeCoords.x + 1) * stride, (nodeCoords.y + 1) * stride, (nodeCoords.z + 1) * stride);

		m_caseIndex = 0;
		if (m_values[CHILD_OFFSET_BK_BOT_L] >= isoLevel) m_caseIndex |= 1;
		if (m_values[CHILD_OFFSET_BK_BOT_R] >= isoLevel) m_caseIndex |= 2;
		if (m_values[CHILD_OFFSET_FR_BOT_R] >= isoLevel) m_caseIndex |= 4;
		if (m_values[CHILD_OFFSET_FR_BOT_L] >= isoLevel) m_caseIndex |= 8;
		if (m_values[CHILD_OFFSET_BK_TOP_L] >= isoLevel) m_caseIndex |= 16;
		if (m_values[CHILD_OFFSET_BK_TOP_R] >= isoLevel) m_caseIndex |= 32;
		if (m_values[CHILD_OFFSET_FR_TOP_R] >= isoLevel) m_caseIndex |= 64;
		if (m_values[CHILD_OFFSET_FR_TOP_L] >= isoLevel) m_caseIndex |= 128;
	}

	// Update state with parent node
	OctreeLayerNode* parent;
	if (id != 0 && layer->AttemptNodeFetch(GetParentID(), parent, true))
		parent->SetChildFlag(GetOffsetAsChild(), true);
}

void OctreeLayerNode::OnSafeDestroy()
{
	// Update state with parent node
	OctreeLayerNode* parent;
	if (m_id != 0 && m_layer->AttemptNodeFetch(GetParentID(), parent, false))
		parent->SetChildFlag(GetOffsetAsChild(), false);
}

void OctreeLayerNode::Push(const uint32& corner, const LayeredVolume* volume, const float& value)
{
	m_values[corner] = value;

	// Update case index
	const float isoLevel = volume->GetIsoLevel();

	// Set bit to 1
	if (value >= isoLevel)
	{
		if		(corner == CHILD_OFFSET_BK_BOT_L) m_caseIndex |= 1;
		else if (corner == CHILD_OFFSET_BK_BOT_R) m_caseIndex |= 2;
		else if (corner == CHILD_OFFSET_FR_BOT_R) m_caseIndex |= 4;
		else if (corner == CHILD_OFFSET_FR_BOT_L) m_caseIndex |= 8;
		else if (corner == CHILD_OFFSET_BK_TOP_L) m_caseIndex |= 16;
		else if (corner == CHILD_OFFSET_BK_TOP_R) m_caseIndex |= 32;
		else if (corner == CHILD_OFFSET_FR_TOP_R) m_caseIndex |= 64;
		else if (corner == CHILD_OFFSET_FR_TOP_L) m_caseIndex |= 128;
		else { LOG_ERROR("Invalid corner used in push"); }
	}
	// Set bit to 0
	else
	{
		if		(corner == CHILD_OFFSET_BK_BOT_L) m_caseIndex &= ~1;
		else if (corner == CHILD_OFFSET_BK_BOT_R) m_caseIndex &= ~2;
		else if (corner == CHILD_OFFSET_FR_BOT_R) m_caseIndex &= ~4;
		else if (corner == CHILD_OFFSET_FR_BOT_L) m_caseIndex &= ~8;
		else if (corner == CHILD_OFFSET_BK_TOP_L) m_caseIndex &= ~16;
		else if (corner == CHILD_OFFSET_BK_TOP_R) m_caseIndex &= ~32;
		else if (corner == CHILD_OFFSET_FR_TOP_R) m_caseIndex &= ~64;
		else if (corner == CHILD_OFFSET_FR_TOP_L) m_caseIndex &= ~128;
		else { LOG_ERROR("Invalid corner used in push"); }
	}

	// TODO - Calculate and cache edges?
}

void OctreeLayerNode::BuildMesh(const float& isoLevel, MeshBuilderMinimal& builder, const uint32& maxDepthOffset)
{
	// Merge
	if (maxDepthOffset != 0 && RequiresHigherDetail())
	{
		if (m_childFlags == 0)
			return; // Has no children, so just stop meshing here

		std::array<OctreeLayerNode*, 8> children;
		FetchChildren(children);


		uint8 currentFlags = 0;
		uint32 i = 0;
		for (OctreeLayerNode* child : children)
		{
			if (child)
			{
				child->BuildMesh(isoLevel, builder, maxDepthOffset - 1);
				currentFlags |= (1 << i);

				if (m_values[i] != child->m_values[i])
				{
					int n = 0;
					n++;
				}
			}
			++i;
		}

		if (currentFlags != m_childFlags)
			i++;

		return;
	}


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

void OctreeLayerNode::FetchChildren(std::array<OctreeLayerNode*, 8>& outList) const 
{
	// Retrieve all children
	for (uint32 i = 0; i < 8; ++i)
	{
		if (!GetChildFlag(i) || !m_layer->AttemptNodeFetch(GetChildID(i), outList[i], false))
			outList[i] = nullptr;
	}
}

bool OctreeLayerNode::IsSimpleCase() const 
{
	if (m_caseIndex == 0 || m_caseIndex == 255)
		return false;

	static std::unordered_set<uint8> simpleCases({ 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 19, 23, 25, 27, 29, 31, 32, 34, 35, 38, 39, 43, 46, 47, 48, 49, 50, 51, 54, 55, 57, 59, 63, 64, 68, 70, 71, 76, 77, 78, 79, 96, 98, 99, 100, 102, 103, 108, 110, 111, 112, 113, 114, 115, 116, 118, 119, 127, 128, 136, 137, 139, 140, 141, 142, 143, 144, 145, 147, 152, 153, 155, 156, 157, 159, 176, 177, 178, 179, 184, 185, 187, 191, 192, 196, 198, 200, 201, 204, 205, 206, 207, 208, 209, 212, 216, 217, 220, 221, 223, 224, 226, 228, 230, 232, 236, 238, 239, 240, 241, 242, 243, 244, 246, 247, 248, 249, 251, 252, 253, 254 });
	return simpleCases.find(m_caseIndex) != simpleCases.end();
}

uint32 OctreeLayerNode::IntersectionCount(const uint32& edge, const uint32& max) const
{
	// Is leaf
	if (m_layer->GetNodeResolution() == 2)
	{
		const uint32 edgeFlag = (1 << edge);
		return (m_caseIndex & edgeFlag) != 0 ? 1 : 0;
	}

	// Count intersection of children on this edge
	else 
	{
		// Gets the ids of the nodes
		static std::array<uvec2, 12> nodeEdgeLookup
		{
			uvec2(0,1),
			uvec2(1,2),
			uvec2(2,3),
			uvec2(0,3),

			uvec2(4,5),
			uvec2(5,6),
			uvec2(6,7),
			uvec2(4,7),

			uvec2(0,4),
			uvec2(1,5),
			uvec2(2,6),
			uvec2(3,7)
		};


		const uvec2& edgeNodes = nodeEdgeLookup[edge];
		OctreeLayerNode* A;
		OctreeLayerNode* B;
		uint32 count = 0;

		// Add first node intersect count
		if (GetChildFlag(edgeNodes.x) && m_layer->AttemptNodeFetch(GetChildID(edgeNodes.x), A, false))
			count += A->IntersectionCount(edge, max);

		if (max != 0 && count >= max)
			return count;

		// Add second node intersect count
		if (GetChildFlag(edgeNodes.y) && m_layer->AttemptNodeFetch(GetChildID(edgeNodes.y), B, false))
			count += B->IntersectionCount(edge, max);

		return count;
	}
}

bool OctreeLayerNode::RequiresHigherDetail() const 
{
	// Lowest allowed size, so cannot merge
	if (m_layer->GetNodeResolution() == 2)
		return false;


	// If this isn't a simple case, it cannot be merged
	if (!IsSimpleCase())
		return true;


	// Check children are simple cases
	OctreeLayerNode* children[8];
	for (uint32 i = 0; i < 8; ++i)
	{
		if (!GetChildFlag(i) || !m_layer->AttemptNodeFetch(GetChildID(i), children[i], false))
			children[i] = nullptr;

		// Check child is simple case
		else if (!children[i]->IsSimpleCase())
			return true;
	}


	// Check for multiple intersections on this nodes edge
	if (IntersectionCount(0, 2) > 1) return true;
	if (IntersectionCount(1, 2) > 1) return true;
	if (IntersectionCount(2, 2) > 1) return true;
	if (IntersectionCount(3, 2) > 1) return true;
	if (IntersectionCount(4, 2) > 1) return true;
	if (IntersectionCount(5, 2) > 1) return true;
	if (IntersectionCount(6, 2) > 1) return true;
	if (IntersectionCount(7, 2) > 1) return true;
	if (IntersectionCount(8, 2) > 1) return true;
	if (IntersectionCount(9, 2) > 1) return true;
	if (IntersectionCount(10, 2) > 1) return true;
	if (IntersectionCount(11, 2) > 1) return true;

	return false;
}

///
/// Layer
///

OctreeLayer::OctreeLayer(LayeredVolume* volume, const uint32& depth, const uint32& height, const uint32& nodeRes) :
	m_volume(volume),
	m_depth(depth), m_nodeResolution(nodeRes), m_layerResolution((volume->GetOctreeResolution() - 1) / GetStride() + 1),
	m_startIndex(depth == 0 ? 0 : (pow(8, depth) - 1) / 7), // Total nodes for a given height is (8^h - 1)/7
	m_endIndex(m_startIndex + (uint32)pow(8U, depth) - 1)
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
	return true;
}

void OctreeLayer::PushValueOntoNode(const uvec3& localCoords, const ivec3& offset, const float& value)
{
	uvec3 nodeCoords = uvec3(localCoords.x + offset.x, localCoords.y + offset.y, localCoords.z + offset.z);
	const uint32 id = GetID(nodeCoords.x, nodeCoords.y, nodeCoords.z);



	// If id is outside of node range of this layer offset has overflowed
	if (id < m_startIndex || id > m_endIndex)
		return;


	// Find node
	OctreeLayerNode* node;
	const bool shouldCreateNode = (value >= m_volume->GetIsoLevel());
	if (AttemptNodeFetch(id, node, shouldCreateNode)) 
	{
		const uint32 corner = -offset.x + 2 * (-offset.y + 2 * -offset.z); // The id of the corner of this value
		node->Push(corner, m_volume, value);

		// Node is now fully out the surface and has no children, so remove it
		if (node->FlaggedForDeletion())
		{
			m_nodes.erase(id);
			node->OnSafeDestroy();
			delete node;
		}
	}
}

void OctreeLayer::BuildMesh(MeshBuilderMinimal& builder)
{
	for (auto node : m_nodes)
		node.second->BuildMesh(m_volume->GetIsoLevel(), builder, 1);
}

bool OctreeLayer::AttemptNodeFetch(const uint32& id, OctreeLayerNode*& outNode, const bool& createIfAbsent)
{
	// Node in previous layer
	if (id < m_startIndex)
	{
		if (previousLayer)
			return previousLayer->AttemptNodeFetch(id, outNode, createIfAbsent);
		else
			return false;
	}
	// Node in next layer
	else if (id > m_endIndex)
	{
		if (nextLayer)
			return nextLayer->AttemptNodeFetch(id, outNode, createIfAbsent);
		else
			return false;
	}
	// Node must be in this layer
	else
	{
		auto it = m_nodes.find(id);
		if (it == m_nodes.end())
		{
			// Make the node, as it currently doesn't exist
			if (createIfAbsent)
			{
				outNode = new OctreeLayerNode(id, this);
				m_nodes[id] = outNode;
				return true;
			}
			else
				return false;
		}
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

///
/// Object functions
///

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
		//m_layers[m_layers.size() - 1]->BuildMesh(builder);
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
	const uint32 layerCount = 2;// 5; // Check layer count is not greater than maxDepth
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