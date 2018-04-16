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

void OctRepNode::GeneratePartialMesh(const float& isoLevel, RetrieveEdgeCallback edgeCallback, VoxelPartialMeshData* target)
{
	if (!IsLeaf() && RequiresHigherDetail())
	{
		for (OctRepNode* child : m_children)
			if (child != nullptr)
				child->GeneratePartialMesh(isoLevel, edgeCallback, target);
		return;
	}

	// Cache values
	const uint32 width = m_resolution - 1;
	float v000 = m_values[GetIndex(0, 0, 0)];
	float v001 = m_values[GetIndex(0, 0, 1)];
	float v010 = m_values[GetIndex(0, 1, 0)];
	float v011 = m_values[GetIndex(0, 1, 1)];
	float v100 = m_values[GetIndex(1, 0, 0)];
	float v101 = m_values[GetIndex(1, 0, 1)];
	float v110 = m_values[GetIndex(1, 1, 0)];
	float v111 = m_values[GetIndex(1, 1, 1)];
	/*
	float v000 = isoValueCallback(m_offset.x + 0 * width, m_offset.y + 0 * width, m_offset.z + 0 * width);
	float v001 = isoValueCallback(m_offset.x + 0 * width, m_offset.y + 0 * width, m_offset.z + 1 * width);
	float v010 = isoValueCallback(m_offset.x + 0 * width, m_offset.y + 1 * width, m_offset.z + 0 * width);
	float v011 = isoValueCallback(m_offset.x + 0 * width, m_offset.y + 1 * width, m_offset.z + 1 * width);
	float v100 = isoValueCallback(m_offset.x + 1 * width, m_offset.y + 0 * width, m_offset.z + 0 * width);
	float v101 = isoValueCallback(m_offset.x + 1 * width, m_offset.y + 0 * width, m_offset.z + 1 * width);
	float v110 = isoValueCallback(m_offset.x + 1 * width, m_offset.y + 1 * width, m_offset.z + 0 * width);
	float v111 = isoValueCallback(m_offset.x + 1 * width, m_offset.y + 1 * width, m_offset.z + 1 * width);
	*/

	// Calculate the current case
	uint8 caseIndex = 0;
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


	// Cache vars
	const vec3 worldCoord = m_offset;
	const uint32 stride = GetResolution() - 1;


	// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) edgeCallback(isoLevel, m_offset + uvec3(x0, y0, z0) * stride, m_offset + uvec3(x1, y1, z1) * stride, m_resolution)//MC::VertexLerp(isoLevel, worldCoord + vec3(x0, y0, z0)*resf, worldCoord + vec3(x1, y1, z1)*resf, v ## x0 ## y0 ## z0, v ## x1 ## y1 ## z1);
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

bool OctRepNode::RequiresHigherDetail() const 
{
	return GetStdDeviation() > 0.115f;
}


///
/// Octree layer
///
void OctreeRepLayer::OnNewNode(OctRepNode* node) 
{
	m_nodes[node->GetOffset()] = node;
}

void OctreeRepLayer::OnModifyNode(OctRepNode* node) 
{
}

void OctreeRepLayer::OnDeleteNode(const OctRepNode* node) 
{
	// TODO - Better impl
	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		if (it->second == node)
		{
			m_nodes.erase(it);
			return;
		}
	}
}

bool OctreeRepLayer::AttemptGet(const uint32& x, const uint32& y, const uint32& z, OctRepNode*& outNode) const 
{
	auto it = m_nodes.find(uvec3(x, y, z));
	if (it == m_nodes.end())
		return false;

	outNode = it->second;
	return true;
}

#include <algorithm>
vec3 OctreeRepLayer::ProjectCorners(const float& isoLevel, const uvec3& a, const uvec3& b, const uvec3& bottomLeft, const uvec3& bottomRight, const uvec3& topLeft, const uvec3& topRight) const
{
	const vec3 highRes = MC::VertexLerp(isoLevel, a, b, m_volume->Get(a.x, a.y, a.z), m_volume->Get(b.x, b.y, b.z));

	const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
	const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
	const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
	const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);

	// Marching Squares to resolve patches
	std::vector<vec3> edges;
	edges.reserve(4);

	// Bottom edge
	if ((vBL >= isoLevel && vBR < isoLevel) || (vBR >= isoLevel && vBL < isoLevel))
		edges.push_back(MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR));
	// Top edge
	if ((vTL >= isoLevel && vTR < isoLevel) || (vTR >= isoLevel && vTL < isoLevel))
		edges.push_back(MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR));

	// Left edge
	if ((vTL >= isoLevel && vBL < isoLevel) || (vBL >= isoLevel && vTL < isoLevel))
		edges.push_back(MC::VertexLerp(isoLevel, topLeft, bottomLeft, vTL, vBL));

	// right edge
	if ((vTR >= isoLevel && vBR < isoLevel) || (vBR >= isoLevel && vTR < isoLevel))
		edges.push_back(MC::VertexLerp(isoLevel, topRight, bottomRight, vTR, vBR));


	std::sort(edges.begin(), edges.end(), [highRes](const vec3& a, const vec3& b) { return glm::distance(highRes, a) < glm::distance(highRes, b); });

	const vec3& A = edges[0];
	const vec3& B = edges[1];
	const vec3& P = highRes;

	const vec3 AB = B - A;
	const vec3 AP = P - A;
	return A + glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
}

vec3 OctreeRepLayer::VertexLerp(const float& isoLevel, const uvec3& a, const uvec3& b) const
{
	const uint32 stride = m_resolution - 1; 
	const uvec3 mid = (a + b) / 2U;
	const uvec3 local = (mid / stride) * stride; // Node that this edge belongs to
	

	// Where it would usually be expected to be
	const vec3 highRes = MC::VertexLerp(isoLevel, a, b, m_volume->Get(a.x, a.y, a.z), m_volume->Get(b.x, b.y, b.z));

	const uvec3 remA = a % stride;
	const uvec3 remB = b % stride;

	const float aVal = m_volume->Get(a.x, a.y, a.z);
	const float bVal = m_volume->Get(b.x, b.y, b.z);
	

	// Edge exists on x axis
	if (a.x != b.x) // Implies a.y == b.y && a.z == b.z
	{
		// Sort to make sure A < B
		const uvec3& A = a.x < b.x ? a : b;
		const uvec3& B = a.x < b.x ? b : a;
		const float& AVal = a.x < b.x ? aVal : bVal;
		const float& BVal = a.x < b.x ? bVal : aVal;

		// Check if is edge of node
		if (remA.y == 0 && remA.z == 0)
		{
			const uvec3 next = local + uvec3(stride, 0, 0);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between y face
		if (remA.y == 0)
		{
			// Check (X,Z)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between z face
		if (remA.z == 0)
		{
			// Check (X,Y)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}

	// Edge exists on y axis
	if (a.y != b.y) // Implies a.x == b.x && a.z == b.z
	{
		// Check if is edge of node
		if (remA.x == 0 && remA.z == 0)
		{
			const uvec3 next = local + uvec3(0, stride, 0);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between x face
		if (remA.x == 0)
		{
			// Check (Y,Z)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between z face
		if (remA.z == 0)
		{
			// Check (Y,X)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}

	// Edge exists on z axis
	if (a.z != b.z) // Implies a.x == b.x && a.y == b.y
	{
		// Check if is edge of node
		if (remA.x == 0 && remA.y == 0)
		{
			const uvec3 next = local + uvec3(0, 0, stride);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between x face
		if (remA.x == 0)
		{
			// Check (Z,Y)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between y face
		if (remA.y == 0)
		{
			// Check (Z,X)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}

	
	return highRes;
}

vec3 OctreeRepLayer::RetrieveEdge(const float& isoLevel, const uvec3& a, const uvec3& b, const uint32& minRes)
{
	// Lowest resolution, so just return normal edge
	if (m_resolution <= minRes)
		return MC::VertexLerp(isoLevel, a, b, m_volume->Get(a.x, a.y, a.z), m_volume->Get(b.x, b.y, b.z));

	return VertexLerp(isoLevel, a, b);


	const float aVal = m_volume->Get(a.x, a.y, a.z);
	const float bVal = m_volume->Get(b.x, b.y, b.z);

	const uint32 stride = m_resolution - 1;
	const uvec3 local = (((a + b) / 2U) / stride) * stride; // Node that this edge belongs to

	const uvec3 remA = a % stride;
	const uvec3 remB = b % stride;


	// Edge exists on x axis
	if (a.x != b.x) // Implies a.y == b.y && a.z == b.z
	{
		// Sort to make sure A < B
		const uvec3& A = a.x < b.x ? a : b;
		const uvec3& B = a.x < b.x ? b : a;
		const float& AVal = a.x < b.x ? aVal : bVal;
		const float& BVal = a.x < b.x ? bVal : aVal;

		// Check if is edge of node
		if (remA.y == 0 && remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			const uvec3 next = local + uvec3(stride, 0, 0);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between y face
		if (remA.y == 0)
		{
			// Check (X,Z)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between z face
		if (remA.z == 0)
		{
			// Check (X,Y)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}

	// Edge exists on y axis
	if (a.y != b.y) // Implies a.x == b.x && a.z == b.z
	{
		// Check if is edge of node
		if (remA.x == 0 && remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			const uvec3 next = local + uvec3(0, stride, 0);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between x face
		if (remA.x == 0)
		{
			// Check (Y,Z)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between z face
		if (remA.z == 0)
		{
			// Check (Y,X)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}

	// Edge exists on z axis
	if (a.z != b.z) // Implies a.x == b.x && a.y == b.y
	{
		// Check if is edge of node
		if (remA.x == 0 && remA.y == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			const uvec3 next = local + uvec3(0, 0, stride);
			return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
		}

		// Edge shared between x face
		if (remA.x == 0)
		{
			// Check (Z,Y)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}

		// Edge shared between y face
		if (remA.y == 0)
		{
			// Check (Z,X)
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			return ProjectCorners(isoLevel, a, b, bottomLeft, bottomRight, topLeft, topRight);
		}
	}



	/// Check if edge sits inbetween 2 octree nodes
	// x axis edge 
	/*
	if (a.x != b.x) 
	{
		/// Find out which face it sits on

		// Edge is at intersection, so use low-res edge instead
		if (false && remA.y == 0 && remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (useThisRes)
			{
				const uvec3 next = local + uvec3(stride, 0, 0);
				return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
			}

			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
		}

		// Sits on y face
		else if (false && remA.y == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x, local.y - stride, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (X,Z) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = (a.x < b.x ? aVal > bVal : bVal > aVal);
			const bool useBottom = (useLeft ? vBL > vTL : vBR > vTR);

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A,P) < glm::distance(B,P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Sits on z face
		else if (false && remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x, local.y, local.z - stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (X,Y) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(stride, 0, 0);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = a.x < b.x ? aVal > bVal : bVal > aVal;
			const bool useBottom = useLeft ? vBL > vTL : vBR > vTR;

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A, P) < glm::distance(B, P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Doesn't sit on face (Must be embedded)
		else
			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
	}

	// y axis edge
	else if (false && a.y != b.y)
	{
		/// Find out which face it sits on

		// Edge is at intersection, so use low-res edge instead
		if (remA.x == 0 && remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			if (useThisRes)
			{
				const uvec3 next = local + uvec3(0, stride, 0);
				return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
			}

			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
		}

		// Sits on x face
		else if (remA.x == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x - stride, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (Y,Z) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(0, 0, stride);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = (a.x < b.x ? aVal > bVal : bVal > aVal);
			const bool useBottom = (useLeft ? vBL > vTL : vBR > vTR);

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A, P) < glm::distance(B, P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Sits on z face
		else if (remA.z == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x, local.y, local.z - stride, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (Y,X) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, stride, 0);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, stride, 0);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = (a.x < b.x ? aVal > bVal : bVal > aVal);
			const bool useBottom = (useLeft ? vBL > vTL : vBR > vTR);

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A, P) < glm::distance(B, P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Doesn't sit on face (Must be embedded)
		else
			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
	}

	// z axis edge
	else if (false && a.z != b.z)
	{
		/// Find out which face it sits on


		// Edge is at intersection, so use low-res edge instead
		if (remA.x == 0 && remA.y == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x + 0, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + 0, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + 0, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x + stride, local.y + stride, local.z + 0, node) && !node->RequiresHigherDetail())
				useThisRes = true;


			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			if (useThisRes)
			{
				const uvec3 next = local + uvec3(0, 0, stride);
				return MC::VertexLerp(isoLevel, local, next, m_volume->Get(local.x, local.y, local.z), m_volume->Get(next.x, next.y, next.z));
			}

			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
		}

		// Sits on x face
		else if (remA.x == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x - stride, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (Z,Y) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(0, stride, 0);
			const uvec3 topRight = local + uvec3(0, stride, stride);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = (a.x < b.x ? aVal > bVal : bVal > aVal);
			const bool useBottom = (useLeft ? vBL > vTL : vBR > vTR);

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A, P) < glm::distance(B, P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Sits on y face
		else if (remA.y == 0)
		{
			// Check whether any nodes at this edge are built at this res
			OctRepNode* node;
			bool useThisRes = false;
			if (AttemptGet(local.x, local.y, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;
			else if (AttemptGet(local.x, local.y - stride, local.z, node) && !node->RequiresHigherDetail())
				useThisRes = true;

			// Requires lower res
			if (!useThisRes)
				return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);


			// (Z,X) FACE
			// Project low-res edge onto high-res edge
			const uvec3 bottomLeft = local;
			const uvec3 bottomRight = local + uvec3(0, 0, stride);
			const uvec3 topLeft = local + uvec3(stride, 0, 0);
			const uvec3 topRight = local + uvec3(stride, 0, stride);

			const float vBL = m_volume->Get(bottomLeft.x, bottomLeft.y, bottomLeft.z);
			const float vTL = m_volume->Get(topLeft.x, topLeft.y, topLeft.z);
			const float vBR = m_volume->Get(bottomRight.x, bottomRight.y, bottomRight.z);
			const float vTR = m_volume->Get(topRight.x, topRight.y, topRight.z);


			// Work out which edges this line projects onto
			const bool useLeft = (a.x < b.x ? aVal > bVal : bVal > aVal);
			const bool useBottom = (useLeft ? vBL > vTL : vBR > vTR);

			// Left/Right & Top/Bottom low-res edge
			// Low-res edge will connect sideEdge <-> flatEdge 
			const vec3 sideEdge = useLeft ? MC::VertexLerp(isoLevel, bottomLeft, topLeft, vBL, vTL) : MC::VertexLerp(isoLevel, bottomRight, topRight, vBR, vTR);
			const vec3 flatEdge = useBottom ? MC::VertexLerp(isoLevel, bottomLeft, bottomRight, vBL, vBR) : MC::VertexLerp(isoLevel, topLeft, topRight, vTL, vTR);

			// Project the high-res point onto the low-res edge
			const vec3 highRes = MC::VertexLerp(isoLevel, a, b, aVal, bVal);

			// Vector rejection
			const vec3& A = sideEdge;
			const vec3& B = flatEdge;
			const vec3& P = highRes;

			const vec3 AB = B - A;
			const vec3 AP = P - A;
			return glm::distance(A, P) < glm::distance(B, P) ? A : B;// +glm::dot(AP, AB) / glm::dot(AB, AB) * AB;
		}

		// Doesn't sit on face (Must be embedded)
		else
			return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
	}
	*/
	return nextLayer->RetrieveEdge(isoLevel, a, b, minRes);
}

float OctreeRepLayer::FetchIsoValue(const uint32& x, const uint32& y, const uint32& z) const
{
	// Lowest resolution, so just return the value
	if (m_resolution == 2)
		return m_volume->Get(x, y, z);


	// Find node that contains this point
	const uint32 stride = m_resolution - 1;
	uvec3 local((x / stride) * stride, (y / stride) * stride, (z / stride) * stride);

	// Can't find node containing this coord, so must be empty
	auto it = m_nodes.find(local);
	if (it == m_nodes.end())
		return DEFAULT_VALUE;


	// Check the next layer
	if (it->second->RequiresHigherDetail() && false)
		return nextLayer->FetchIsoValue(x, y, z);

	// Corner value
	else if(x % stride == 0 && y % stride == 0 && z % stride == 0)
		return m_volume->Get(x, y, z);

	// Interpolate corners to get value
	else
	{
		float xf = (x % m_resolution) / m_resolution;
		float yf = (y % m_resolution) / m_resolution;
		float zf = (z % m_resolution) / m_resolution;

		const float v000 = m_volume->Get(local.x + 0 * stride, local.y + 0 * stride, local.z + 0 * stride);
		const float v001 = m_volume->Get(local.x + 0 * stride, local.y + 0 * stride, local.z + 1 * stride);
		const float v010 = m_volume->Get(local.x + 0 * stride, local.y + 1 * stride, local.z + 0 * stride);
		const float v011 = m_volume->Get(local.x + 0 * stride, local.y + 1 * stride, local.z + 1 * stride);
		const float v100 = m_volume->Get(local.x + 1 * stride, local.y + 0 * stride, local.z + 0 * stride);
		const float v101 = m_volume->Get(local.x + 1 * stride, local.y + 0 * stride, local.z + 1 * stride);
		const float v110 = m_volume->Get(local.x + 1 * stride, local.y + 1 * stride, local.z + 0 * stride);
		const float v111 = m_volume->Get(local.x + 1 * stride, local.y + 1 * stride, local.z + 1 * stride);

		// Lerp along axis
		const float vX00 = v000 * (1.0f - xf) + v100 * xf;
		const float vX10 = v010 * (1.0f - xf) + v110 * xf;
		const float vX01 = v001 * (1.0f - xf) + v101 * xf;
		const float vX11 = v011 * (1.0f - xf) + v111 * xf;

		const float vY0 = vX00 * (1.0f - yf) + vX10 * yf;
		const float vY1 = vX01 * (1.0f - yf) + vX11 * yf;

		//return volume->Get(x, y, z);
		return vY0 * (1.0f - zf) + vY1 * zf;
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
		res = 1 + pow(2U, i + 1);

	LOG("OctreeRepVolume using normal_res: (%i,%i,%i) octree_res: (%i,%i,%i)", resolution.x, resolution.y, resolution.z, res, res, res);

	m_resolution = resolution;
	m_scale = scale;
	m_data.clear();
	m_data.resize(resolution.x * resolution.y * resolution.z);

	m_octree = new OctRepNode(res);
	m_layers.clear();

	// Setup layers
	m_layers.emplace_back(this, 1U + (uint32)pow(2, 3));
	m_layers.emplace_back(this, 1U + (uint32)pow(2, 2));
	m_layers.emplace_back(this, 1U + (uint32)pow(2, 1));
	m_layers.emplace_back(this, 1U + (uint32)pow(2, 0));
	for (uint32 i = 0; i < m_layers.size() - 1; ++i)
		m_layers[i].nextLayer = &m_layers[i + 1];
}

void OctreeRepVolume::Set(uint32 x, uint32 y, uint32 z, float value)
{
	m_data[GetIndex(x, y, z)] = value;

	OctRepNotifyPacket changes;
	m_octree->Push(x, y, z, value, changes);


	// Go through layers to see if any are interested
	{
		// New
		for (OctRepNode* node : changes.newNodes)
			for (OctreeRepLayer& layer : m_layers)
				if (node->GetResolution() == layer.GetResolution())
					layer.OnNewNode(node);

		// Updated
		for (OctRepNode* node : changes.updatedNodes)
			for (OctreeRepLayer& layer : m_layers)
				if (node->GetResolution() == layer.GetResolution())
					layer.OnModifyNode(node);

		// Deleted
		for (OctRepNode* node : changes.deletedNodes)
			for (OctreeRepLayer& layer : m_layers)
				if (node->GetResolution() == layer.GetResolution())
					layer.OnDeleteNode(node);
	}


	// TEMP MESH DATA STUFF
	// Remove deleted nodes
	for (OctRepNode* node : changes.deletedNodes)
	{
		auto it = m_nodeLevel.find(node);
		if (it != m_nodeLevel.end())
			m_nodeLevel.erase(it);
	}

	// Add leaf nodes to list
	for (OctRepNode* node : changes.newNodes)
		if (node->GetResolution() == m_layers[0].GetResolution())
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
	if (x >= m_resolution.x || y >= m_resolution.y || z >= m_resolution.z)
		return UNKNOWN_BUILD_VALUE;

	return m_data[GetIndex(x, y, z)];
}

float OctreeRepVolume::Get(uint32 x, uint32 y, uint32 z) const
{
	if (x >= m_resolution.x || y >= m_resolution.y || z >= m_resolution.z)
		return UNKNOWN_BUILD_VALUE;

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
			using namespace std::placeholders;
			pair.first->GeneratePartialMesh(m_isoLevel, std::bind(&OctreeRepLayer::RetrieveEdge, &m_layers[0], _1, _2, _3, _4), &pair.second);
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

float OctreeRepVolume::GetIsoValue(const uint32& x, const uint32& y, const uint32& z) const
{
	if (x >= m_resolution.x || y >= m_resolution.y || z >= m_resolution.z)
		return UNKNOWN_BUILD_VALUE;

	return m_layers[0].FetchIsoValue(x, y, z);
}