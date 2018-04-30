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
	m_id(id), m_values({ DEFAULT_VALUE }), m_layer(layer), m_childFlags(0)
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
		if (m_values[CHILD_OFFSET_BK_BOT_L] > isoLevel) m_caseIndex |= 1;
		if (m_values[CHILD_OFFSET_BK_BOT_R] > isoLevel) m_caseIndex |= 2;
		if (m_values[CHILD_OFFSET_FR_BOT_R] > isoLevel) m_caseIndex |= 4;
		if (m_values[CHILD_OFFSET_FR_BOT_L] > isoLevel) m_caseIndex |= 8;
		if (m_values[CHILD_OFFSET_BK_TOP_L] > isoLevel) m_caseIndex |= 16;
		if (m_values[CHILD_OFFSET_BK_TOP_R] > isoLevel) m_caseIndex |= 32;
		if (m_values[CHILD_OFFSET_FR_TOP_R] > isoLevel) m_caseIndex |= 64;
		if (m_values[CHILD_OFFSET_FR_TOP_L] > isoLevel) m_caseIndex |= 128;

		RecalculateStats();
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
	const float oldValue = m_values[corner];
	m_values[corner] = value;

	// Update case index
	const uint32 oldCase = m_caseIndex;
	const float isoLevel = volume->GetIsoLevel();

	// Set bit to 1
	if (value > isoLevel)
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

	// Only notify for rebuild if note worty change happens
	if (oldCase != m_caseIndex || (value != oldValue && (m_caseIndex != 0 || m_caseIndex != 255)))
	{
		m_layer->rebuildFlag = true;
		RecalculateStats();
	}

}

uint32 OctreeLayerNode::GetOffsetAsChild() const
{
	uint32 offset = 0;
	const uint32 i = m_id - m_layer->GetStartID();
	const uint32 res = m_layer->GetLayerResolution() - 1;

	if ((i % 2) != 0) offset |= 1;
	if (((i / (res)) % 2) != 0) offset |= 2;
	if (((i / (res * res)) % 2) != 0) offset |= 4;

	return offset;
}

uint32 OctreeLayerNode::GetParentID() const
{
	if (!m_layer->previousLayer)
		return 0;

	// Fetch the id of the Bottom Left Corner this node is contained in
	const uint32 i = m_id - m_layer->GetStartID();
	const uint32 thisRes = m_layer->GetLayerResolution() - 1;
	const uint32 parentRes = thisRes / 2;
	const uint32 offset = GetOffsetAsChild();

	uint32 n = i;
	if (offset & 1) n -= 1; // Offset X
	if (offset & 2) n -= thisRes; // Offset Y
	if (offset & 4) n -= thisRes * thisRes; // Offset Z

	
	return m_layer->previousLayer->GetStartID() +
		(n % thisRes) / 2 +
		(((n % (thisRes * thisRes)) / thisRes) / 2) * parentRes +
		(((n % (thisRes * thisRes * thisRes)) / (thisRes * thisRes)) / 2) * parentRes*parentRes;
}

uint32 OctreeLayerNode::GetChildID(const uint32& offset) const
{
	const uint32 i = m_id - m_layer->GetStartID();
	const uint32 res = m_layer->GetLayerResolution() - 1;

	// Calculate the position of the Back Bottom Left
	const uint32 childRes = res * 2;

	const uint32 c000 =
		(i % res) * 2 +
		((i / res) % res) * 2 * childRes +
		(i / (res * res)) * 2 * childRes * childRes;

	uint32 cId = c000;
	if (offset & 1) cId += 1; // Offset X
	if (offset & 2) cId += childRes; // Offset Y
	if (offset & 4) cId += childRes * childRes; // Offset Z

	return m_layer->GetEndID() + 1 + cId ;
}

static bool buildDebugMesh = false;
void OctreeLayerNode::BuildMesh(const float& isoLevel, MeshBuilderMinimal& builder, const uint32& maxDepthOffset, OctreeLayer* highestLayer, const uint32& highestLayerOffset)
{
	// Merge
	if (maxDepthOffset != 0 && RequiresHigherDetail(maxDepthOffset))
	{
		if (m_childFlags == 0)
			return; // Has no children, so just stop meshing here

		std::array<OctreeLayerNode*, 8> children;
		FetchChildren(children);
		
		for (OctreeLayerNode* child : children)
		{
			if (child)
				child->BuildMesh(isoLevel, builder, maxDepthOffset - 1, highestLayer, highestLayerOffset);
		}
		
		return;
	}


	if (m_caseIndex == 0 || m_caseIndex == 255)
		return;

	const uint32 stride = m_layer->GetStride();
	const uvec3 layerCoords = m_layer->GetLocalCoords(m_id);
	const float stridef = stride;

	if (buildDebugMesh)
	{
		uint32 childCount = 0;
		if (m_childFlags & 1) childCount++;
		if (m_childFlags & 2) childCount++;
		if (m_childFlags & 4) childCount++;
		if (m_childFlags & 8) childCount++;
		if (m_childFlags & 16) childCount++;
		if (m_childFlags & 64) childCount++;
		if (m_childFlags & 128) childCount++;
		if (m_childFlags & 256) childCount++;

		const vec3 normal = (m_id == 1454 ? vec3(0, -1, 0) : vec3(0, 1, 0));
		//TODO - Remove

		const uint32 i0 = builder.AddVertex((vec3(layerCoords) + vec3(0.0f, 0.0f, 0.0f)) * stridef, normal);
		const uint32 i1 = builder.AddVertex((vec3(layerCoords) + vec3(1.0f, 0.0f, 0.0f)) * stridef, normal);
		const uint32 i2 = builder.AddVertex((vec3(layerCoords) + vec3(0.0f, 0.0f, 1.0f)) * stridef, normal);
		const uint32 i3 = builder.AddVertex((vec3(layerCoords) + vec3(1.0f, 0.0f, 1.0f)) * stridef, normal);

		const uint32 i4 = builder.AddVertex((vec3(layerCoords) + vec3(0.0f, 1.0f, 0.0f)) * stridef, normal);
		const uint32 i5 = builder.AddVertex((vec3(layerCoords) + vec3(1.0f, 1.0f, 0.0f)) * stridef, normal);
		const uint32 i6 = builder.AddVertex((vec3(layerCoords) + vec3(0.0f, 1.0f, 1.0f)) * stridef, normal);
		const uint32 i7 = builder.AddVertex((vec3(layerCoords) + vec3(1.0f, 1.0f, 1.0f)) * stridef, normal);

		builder.AddTriangle(i0, i1, i2); builder.AddTriangle(i2, i1, i3);
		builder.AddTriangle(i4, i6, i5); builder.AddTriangle(i5, i6, i7);
		builder.AddTriangle(i0, i2, i1); builder.AddTriangle(i2, i3, i1);
		builder.AddTriangle(i4, i5, i6); builder.AddTriangle(i5, i7, i6);

		builder.AddTriangle(i2, i3, i6); builder.AddTriangle(i6, i3, i7);
		builder.AddTriangle(i3, i1, i7); builder.AddTriangle(i1, i5, i7);
		builder.AddTriangle(i2, i6, i3); builder.AddTriangle(i6, i7, i3);
		builder.AddTriangle(i3, i7, i1); builder.AddTriangle(i1, i7, i5);

		builder.AddTriangle(i1, i0, i4); builder.AddTriangle(i1, i4, i5);
		builder.AddTriangle(i0, i2, i4); builder.AddTriangle(i2, i6, i4);
		builder.AddTriangle(i1, i4, i0); builder.AddTriangle(i1, i5, i4);
		builder.AddTriangle(i0, i4, i2); builder.AddTriangle(i2, i4, i6);
		return;
	}
	

	// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) highestLayer->OverrideEdge(vec3(layerCoords + uvec3(x0,y0,z0)) * stridef, vec3(layerCoords + uvec3(x1,y1,z1)) * stridef, highestLayerOffset, temp) ? temp : MC::VertexLerp(isoLevel, vec3(layerCoords + uvec3(x0,y0,z0)) * stridef, vec3(layerCoords + uvec3(x1,y1,z1)) * stridef, m_values[GetIndex(x0, y0, z0)], m_values[GetIndex(x1, y1, z1)])
//#define VERT_LERP(x0, y0, z0, x1, y1, z1) highestLayer->OverrideEdge(vec3(layerCoords + uvec3(x0,y0,z0)) * stridef, vec3(layerCoords + uvec3(x1,y1,z1)) * stridef, highestLayerOffset, temp) ? temp : MC::VertexLerp(isoLevel, vec3(layerCoords + uvec3(x0,y0,z0)) * stridef, vec3(layerCoords + uvec3(x1,y1,z1)) * stridef, m_layer->GetVolume()->Get((layerCoords.x + x0) * stride, (layerCoords.y + y0) * stride, (layerCoords.z + z0) * stride), m_layer->GetVolume()->Get((layerCoords.x + x1) * stride, (layerCoords.y + y1) * stride, (layerCoords.z + z1) * stride))
	vec3 temp;
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

		const vec3& A = edges[edge0];
		const vec3& B = edges[edge1];
		const vec3& C = edges[edge2];

		// Ignore triangle which is malformed
		if (A == B || A == C || B == C)
			continue;


		const vec3 normal = glm::cross(B - A, C - A);
		const float normalLengthSqrd = dot(normal, normal);

		// If normal is 0 it means the edge has been moved so the face is now a line
		if (normalLengthSqrd != 0.0f && !std::isnan(normalLengthSqrd))
		{
			const uint32 a = builder.AddVertex(A, normal);
			const uint32 b = builder.AddVertex(B, normal);
			const uint32 c = builder.AddVertex(C, normal);
			builder.AddTriangle(a, b, c);
		}
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

void OctreeLayerNode::RecalculateStats()
{
	RecalculateMergeDepth();

	// Notify parent
	OctreeLayerNode* parent;
	if (m_id != 0 && m_layer->AttemptNodeFetch(GetParentID(), parent, false))
		parent->RecalculateStats();

	/*
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
	*/
}

void OctreeLayerNode::RecalculateMergeDepth()
{
	// Recalculate how deep this node can go before loosing too much detail (Assumes children are correct)
	m_safeQualityDepth = 0;

	// Not point doing further checks if there is not children
	if (m_childFlags == 0)
		return;

	// Lowest allowed size, so cannot merge
	if (m_layer->GetNodeResolution() == 2)
		return;


	// Check if children require more detail
	std::array<OctreeLayerNode*, 8> children;
	FetchChildren(children);

	// Check children are all simple case
	bool childrenSimple = true;
	for (OctreeLayerNode* child : children)
		if (child && !child->IsSimpleCase()) 
		{
			childrenSimple = false;
			break;
		}

	// It is safe to merge this node at this height
	if (childrenSimple && IsSimpleCase() && !HasMultipleIntersections())
		return;

	// Get the smallest merge depth from the children
	m_safeQualityDepth = 255;
	for (OctreeLayerNode* child : children)
		if (child)
		{
			const uint8 depth = child->m_safeQualityDepth + 1;

			if(depth < m_safeQualityDepth)
				m_safeQualityDepth = depth;
		}

	// TODO - Check for multi
	// If the values deviate by too much, merge them
	//return m_average < m_layer->GetVolume()->GetIsoLevel() && m_stdDeviation > 0.1;
}

bool OctreeLayerNode::IsSimpleCase() const 
{	
	static std::unordered_set<uint8> simpleCases({ 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 19, 20, 23, 25, 27, 29, 31, 32, 34, 35, 38, 39, 40, 43, 46, 47, 48, 49, 50, 51, 54, 55, 57, 59, 63, 64, 65, 68, 70, 71, 76, 77, 78, 79, 96, 98, 99, 100, 102, 103, 108, 110, 111, 112, 113, 114, 115, 116, 118, 119, 125, 127, 128, 130, 136, 137, 139, 140, 141, 142, 143, 144, 145, 147, 152, 153, 155, 156, 157, 159, 176, 177, 178, 179, 184, 185, 187, 190, 191, 192, 196, 198, 200, 201, 204, 205, 206, 207, 208, 209, 212, 215, 216, 217, 220, 221, 223, 224, 226, 228, 230, 232, 235, 236, 238, 239, 240, 241, 242, 243, 244, 246, 247, 248, 249, 251, 252, 253, 254, });
	return simpleCases.find(m_caseIndex) != simpleCases.end();
}

bool OctreeLayerNode::HasMultipleIntersections() const 
{
	//*
	// Check all possible edges inside and outside this node (For it's children)
	std::array<OctreeLayerNode*, 8> children;
	FetchChildren(children);

#define CHECK_EDGE(child0, child1, edge) if(children[child0] && children[child1] && (MC::CaseRequiredEdges[children[child0]->m_caseIndex] & (1 << edge)) != 0 && (MC::CaseRequiredEdges[children[child1]->m_caseIndex] & (1 << edge)) != 0) return true;

	// Check X dir
	{
		CHECK_EDGE(0, 1, 0);
		CHECK_EDGE(0, 1, 2);
		CHECK_EDGE(0, 1, 6); // Inside
		CHECK_EDGE(0, 1, 4);

		CHECK_EDGE(2, 3, 0);
		CHECK_EDGE(2, 3, 2);
		CHECK_EDGE(2, 3, 6);
		CHECK_EDGE(2, 3, 4); // Inside

		CHECK_EDGE(7, 6, 0); // Inside
		CHECK_EDGE(7, 6, 2);
		CHECK_EDGE(7, 6, 6);
		CHECK_EDGE(7, 6, 4);

		CHECK_EDGE(4, 5, 0);
		CHECK_EDGE(4, 5, 2); // Inside
		CHECK_EDGE(4, 5, 6);
		CHECK_EDGE(4, 5, 4);
	}

	// Check Y dir
	{
		CHECK_EDGE(0, 4, 8);
		CHECK_EDGE(0, 4, 9);
		CHECK_EDGE(0, 4, 10); // Inside
		CHECK_EDGE(0, 4, 11);

		CHECK_EDGE(1, 5, 8);
		CHECK_EDGE(1, 5, 9);
		CHECK_EDGE(1, 5, 10);
		CHECK_EDGE(1, 5, 11); // Inside

		CHECK_EDGE(2, 6, 8); // Inside
		CHECK_EDGE(2, 6, 9);
		CHECK_EDGE(2, 6, 10);
		CHECK_EDGE(2, 6, 11);

		CHECK_EDGE(3, 7, 8);
		CHECK_EDGE(3, 7, 9); // Inside
		CHECK_EDGE(3, 7, 10);
		CHECK_EDGE(3, 7, 11);
	}

	// Check Z dir
	{
		CHECK_EDGE(0, 3, 3);
		CHECK_EDGE(0, 3, 1);
		CHECK_EDGE(0, 3, 5); // Inside
		CHECK_EDGE(0, 3, 7);

		CHECK_EDGE(1, 2, 3);
		CHECK_EDGE(1, 2, 1);
		CHECK_EDGE(1, 2, 5);
		CHECK_EDGE(1, 2, 7); // Inside

		CHECK_EDGE(5, 6, 3); // Inside
		CHECK_EDGE(5, 6, 1);
		CHECK_EDGE(5, 6, 5);
		CHECK_EDGE(5, 6, 7);

		CHECK_EDGE(4, 7, 3);
		CHECK_EDGE(4, 7, 1); // Inside
		CHECK_EDGE(4, 7, 5);
		CHECK_EDGE(4, 7, 7);
	}
	
	for (OctreeLayerNode* child : children)
		if (child && child->HasMultipleIntersections())
			return true;

	return false;
}

bool OctreeLayerNode::RequiresHigherDetail(const uint32& maxDepthOffset) const
{
	if (m_safeQualityDepth == 0 || maxDepthOffset == 0)
		return false;

	return (m_safeQualityDepth >= maxDepthOffset);
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
	for (const auto& pair : m_nodes)
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

bool OctreeLayer::PushValueOntoNode(const uvec3& localCoords, const ivec3& offset, float value)
{
	uvec3 nodeCoords = uvec3(localCoords.x + offset.x, localCoords.y + offset.y, localCoords.z + offset.z);
	if ((localCoords.x == 0 && offset.x < 0) || (localCoords.y == 0 && offset.y < 0) || (localCoords.z == 0 && offset.z < 0))
		return false; // Requested coords out of range

	const uint32 id = GetID(nodeCoords.x, nodeCoords.y, nodeCoords.z);

	// If id is outside of node range of this layer offset has overflowed
	if (id < m_startIndex || id > m_endIndex)
		return false;


	// Find node
	OctreeLayerNode* node;
	const bool shouldCreateNode = (value > m_volume->GetIsoLevel());
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
			return false;
		}

		return true;
	}

	return false;
}

bool OctreeLayer::BuildMesh(MeshBuilderMinimal& builder, const uint32& maxDepthOffset)
{
	// Check lower layers to see if need to rebuild
	if (!rebuildFlag)
	{
		OctreeLayer* layer = nextLayer;

		for (uint32 i = 0; i < maxDepthOffset; ++i) 
		{
			if (layer == nullptr)
				break;
			if (layer->rebuildFlag)
			{
				rebuildFlag = true;
				break;
			}
			layer = layer->nextLayer;
		}
	}

	// Force build to happen
	if (rebuildFlag)
	{
		for (const auto& node : m_nodes)
			node.second->BuildMesh(m_volume->GetIsoLevel(), builder, maxDepthOffset, this, maxDepthOffset);
		rebuildFlag = false;
		return true;
	}
	return false;
}

bool OctreeLayer::ProjectEdgeOntoFace(const uvec3& a, const uvec3& b, const uint32& maxDepthOffset, vec3& overrideOutput, const uvec3& c00, const uvec3& c01, const uvec3& c10, const uvec3& c11) const
{
	const float isolevel = m_volume->GetIsoLevel();

	// Fetch values
	const float v00 = m_volume->Get(c00.x, c00.y, c00.z);
	const float v01 = m_volume->Get(c01.x, c01.y, c01.z);
	const float v10 = m_volume->Get(c10.x, c10.y, c10.z);
	const float v11 = m_volume->Get(c11.x, c11.y, c11.z);

	// Are values inside or isosurface
	const bool b00 = (v00 > isolevel);
	const bool b01 = (v01 > isolevel);
	const bool b10 = (v10 > isolevel);
	const bool b11 = (v11 > isolevel);

	// Retrieve edges
	std::vector<vec3> edges;

	if (b00 != b01)
		edges.push_back(MC::VertexLerp(isolevel, c00, c01, v00, v01));
	if (b00 != b10)
		edges.push_back(MC::VertexLerp(isolevel, c00, c10, v00, v10));
	if (b11 != b01)
		edges.push_back(MC::VertexLerp(isolevel, c11, c01, v11, v01));
	if (b11 != b10)
		edges.push_back(MC::VertexLerp(isolevel, c11, c10, v11, v10));

	
	// Unexpected number of edges, so check next layer
	if (edges.size() != 2)
		return false;// (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
	

	// Project point onto line through vector rejection method
	const vec3& A = edges[0];
	const vec3& B = edges[1];
	const vec3& P = MC::VertexLerp(isolevel, a, b, m_volume->Get(a.x, a.y, a.z), m_volume->Get(b.x, b.y, b.z));

	const vec3 AB = B - A;
	const vec3 AP = P - A;
	overrideOutput = A + (glm::dot(AP, AB) / glm::dot(AB, AB)) * AB;
	return true;
}

bool OctreeLayer::OverrideEdge(const uvec3& a, const uvec3& b, const uint32& maxDepthOffset, vec3& overrideOutput) const
{
	// At lowest depth, so cannot override edge
	if (maxDepthOffset == 0)
		return false;

	// Fetch local coord of average cell this is edge is in
	const uint32 stride = GetStride();
	const uvec3 local = ((a + b) / 2U) / stride;
	const float isolevel = m_volume->GetIsoLevel();

	const uvec3 remA = a % stride;
	const uvec3 remB = b % stride;



	// Moving on x-axis
	if (a.x != b.x) // Imply a.y == b.y && a.z == b.z
	{
		// Is larger than the current stride, so cannot be overridden
		if ((a.x < b.x ? b.x - a.x : a.x - b.x) >= stride)
			return false;
		

		// Edge is entirely encased in this layer
		if (remA.y != 0 && remA.z != 0)
			return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);


		// Edge is inline with edge at this res
		if (remA.y == 0 && remA.z == 0)
		{
			// Check cornering nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(1) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, -1, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(16) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, -1), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(4) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, -1, -1), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(64) && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// One of the nodes is being built at this edge, so use the same value for this edge
				uvec3 layerA = local * stride;
				uvec3 layerB = (local + uvec3(1, 0, 0)) * stride;

				// TODO - Check there is an edge at this res

				overrideOutput = MC::VertexLerp(isolevel, layerA, layerB, m_volume->Get(layerA.x, layerA.y, layerA.z), m_volume->Get(layerB.x, layerB.y, layerB.z));
				return true;
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on y-face 
		else if (remA.y == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, -1, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (X, Z)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(1, 0, 0)) *stride;
				const uvec3 c10 = (local + uvec3(0, 0, 1)) *stride;
				const uvec3 c11 = (local + uvec3(1, 0, 1)) *stride;

				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on z-face 
		else if (remA.z == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, -1), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (X, Y)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(1, 0, 0)) *stride;
				const uvec3 c10 = (local + uvec3(0, 1, 0)) *stride;
				const uvec3 c11 = (local + uvec3(1, 1, 0)) *stride;

				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}
	}


	// Moving on y-axis
	if (a.y != b.y) // Imply a.x == b.x && a.z == b.z
	{
		// Is larger than the current stride, so cannot be overridden
		if ((a.y < b.y ? b.y - a.y : a.y- b.y) >= stride)
			return false;


		// Edge is entirely encased in this layer
		if (remA.x != 0 && remA.z != 0)
			return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);


		// Edge is inline with edge at this res
		if (remA.x == 0 && remA.z == 0)
		{
			// Check cornering nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(256) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(512) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, -1), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(2048) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, 0, -1), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(1024) && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// One of the nodes is being built at this edge, so use the same value for this edge
				uvec3 layerA = local * stride;
				uvec3 layerB = (local + uvec3(0, 1, 0)) * stride;

				// TODO - Check there is an edge at this res

				overrideOutput = MC::VertexLerp(isolevel, layerA, layerB, m_volume->Get(layerA.x, layerA.y, layerA.z), m_volume->Get(layerB.x, layerB.y, layerB.z));
				return true;
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on x-face 
		else if (remA.x == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (Y, Z)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(0, 1, 0)) *stride;
				const uvec3 c10 = (local + uvec3(0, 0, 1)) *stride;
				const uvec3 c11 = (local + uvec3(0, 1, 1)) *stride;

				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on z-face 
		else if (remA.z == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, -1), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (Y, X)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(0, 1, 0)) *stride;
				const uvec3 c10 = (local + uvec3(1, 0, 0)) *stride;
				const uvec3 c11 = (local + uvec3(1, 1, 0)) *stride;

				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}
	}


	// Moving on z-axis
	if (a.z != b.z) // Imply a.x == b.x && a.y == b.y
	{
		// Is larger than the current stride, so cannot be overridden
		if ((a.z < b.z ? b.z - a.z : a.z - b.z) >= stride)
			return false;


		// Edge is entirely encased in this layer
		if (remA.x != 0 && remA.y != 0)
			return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);


		// Edge is inline with edge at this res
		if (remA.x == 0 && remA.y == 0)
		{
			// Check cornering nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(8) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(2) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, -1, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(128) && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, -1, 0), tempNode) && tempNode->GetCaseIndex() != 0 && tempNode->HasEdge(32) && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// One of the nodes is being built at this edge, so use the same value for this edge
				uvec3 layerA = local * stride;
				uvec3 layerB = (local + uvec3(0, 0, 1)) * stride;

				// TODO - Check there is an edge at this res

				overrideOutput = MC::VertexLerp(isolevel, layerA, layerB, m_volume->Get(layerA.x, layerA.y, layerA.z), m_volume->Get(layerB.x, layerB.y, layerB.z));
				return true;
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on x-face 
		else if (remA.x == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(-1, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (Z, Y)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(0, 0, 1)) *stride;
				const uvec3 c10 = (local + uvec3(0, 1, 0)) *stride;
				const uvec3 c11 = (local + uvec3(0, 1, 1)) *stride;

				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}

		// Falls on y-face 
		else if (remA.y == 0)
		{
			// Check nodes for if they are higher res or not
			OctreeLayerNode* tempNode;
			if (
				(AttemptNodeOffsetFetch(local, ivec3(0, 0, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset)) ||
				(AttemptNodeOffsetFetch(local, ivec3(0, -1, 0), tempNode) && tempNode->GetCaseIndex() != 0 && !tempNode->RequiresHigherDetail(maxDepthOffset))
				)
			{
				// (Z, X)
				const uvec3 c00 = (local + uvec3(0, 0, 0)) *stride;
				const uvec3 c01 = (local + uvec3(0, 0, 1)) *stride;
				const uvec3 c10 = (local + uvec3(1, 0, 0)) *stride;
				const uvec3 c11 = (local + uvec3(1, 0, 1)) *stride;

				// KOKO
				return ProjectEdgeOntoFace(a, b, maxDepthOffset, overrideOutput, c00, c01, c10, c11);
			}
			// Edge is not locked to this res
			else
				return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
		}
	}


	// Wasn't overridden at this layer, so pass it to the next
	return (nextLayer != nullptr ? nextLayer->OverrideEdge(a, b, maxDepthOffset - 1, overrideOutput) : false);
}

bool OctreeLayer::AttemptNodeOffsetFetch(const uvec3& localCoords, const ivec3& offset, OctreeLayerNode*& outNode) const
{
	uvec3 nodeCoords = uvec3(localCoords.x + offset.x, localCoords.y + offset.y, localCoords.z + offset.z);
	if ((localCoords.x == 0 && offset.x < 0) || (localCoords.y == 0 && offset.y < 0) || (localCoords.z == 0 && offset.z < 0))
		return false; // Requested coords out of range

	const uint32 id = GetID(nodeCoords.x, nodeCoords.y, nodeCoords.z);

	// If id is outside of node range of this layer offset has overflowed
	if (id < m_startIndex || id > m_endIndex)
		return false;

	
	// Attempt to find the node
	auto it = m_nodes.find(id);
	if (it == m_nodes.end())
		return false;
	else
	{
		outNode = it->second;
		return true;
	}
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
	for (Mesh* mesh : m_meshes)
		delete mesh;

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
}

void LayeredVolume::Update(const float & deltaTime)
{
	if (TEST_REBUILD)
	{
		MeshBuilderMinimal builder;
		m_layers[currentLod]->BuildMesh(builder, lodDepth);
		builder.BuildMesh(m_meshes[currentLod]);

		LOG("Build Count:%i", m_meshes[currentLod]->GetDrawCount());
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


	if (m_meshes[currentLod] != nullptr)
	{
		Transform t;

		if (drawMainObject)
		{
			m_material->Bind(window, GetLevel());
			m_material->PrepareMesh(m_meshes[currentLod]);
			m_material->RenderInstance(&t);
			m_material->Unbind(window, GetLevel());
		}

		if (drawWireFrame)
		{
			m_wireMaterial->Bind(window, GetLevel());
			m_wireMaterial->PrepareMesh(m_meshes[currentLod]);
			m_wireMaterial->RenderInstance(&t);
			m_wireMaterial->Unbind(window, GetLevel());
		}
	}

	// Completely rebuild mesh
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_N))
	{
		buildDebugMesh = false;
		TEST_REBUILD = true;
		for (OctreeLayer* layer : m_layers)
			layer->rebuildFlag = true;

		LOG("Done");
	}
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_M))
	{
		buildDebugMesh = true;
		TEST_REBUILD = true;
		for (OctreeLayer* layer : m_layers)
			layer->rebuildFlag = true;

		LOG("Done");
	}

	if (keyboard->IsKeyPressed(Keyboard::Key::KV_U))
	{
		currentLod++;
		if (currentLod >= m_layers.size())
			currentLod = 0;

		LOG("Level %i", currentLod);
	}
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_I))
	{
		lodDepth++;
		if (lodDepth >= m_layers.size())
			lodDepth = 0;

		LOG("Depth %i", lodDepth);
	}
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
	const uint32 layerCount = 6;// 5; // Check layer count is not greater than maxDepth
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

	// Create meshes
	for (uint32 i = 0; i < m_layers.size(); ++i)
		m_meshes.push_back(new Mesh);

	currentLod = m_layers.size() - 1;
}

void LayeredVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	// Notify any layers of any changes
	for (OctreeLayer* layer : m_layers)
		TEST_REBUILD |= layer->HandlePush(x, y, z, value);

	m_data[GetIndex(x, y, z)] = value;
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

#include <chrono>
using namespace std::chrono;
VoxelBuildResults LayeredVolume::Rebuild(const std::vector<VoxelDelta>& deltas, VoxelBuildResults* recreation) 
{
	// recreation ignored for LayeredVolume
	VoxelBuildResults results;

	results.buildTime.resize(m_layers.size());
	results.tricount.resize(m_layers.size());


	int64 startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	int64 endTime;

	// Insert values
	for (const VoxelDelta& delta : deltas)
	{
		for (OctreeLayer* layer : m_layers)
			layer->HandlePush(delta.coord.x, delta.coord.y, delta.coord.z, delta.value);

		m_data[GetIndex(delta.coord.x, delta.coord.y, delta.coord.z)] = delta.value;
	}
	endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	results.insertTime = endTime - startTime;


	// Rebuild meshes
	for (uint32 i = 0; i < m_layers.size(); ++i)
	{
		int64 buildStartTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

		MeshBuilderMinimal builder;
		builder.MarkDynamic();
		if(m_layers[i]->BuildMesh(builder, lodDepth))
			builder.BuildMesh(m_meshes[i]);
		
		endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		results.buildTime[i] = endTime - buildStartTime;
		results.tricount[i] = m_meshes[i]->GetDrawCount();
	}


	// Total time
	endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	results.totalTime = endTime - startTime;
	return results;
}