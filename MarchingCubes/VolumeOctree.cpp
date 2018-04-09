#include "VolumeOctree.h"
#include "Logger.h"
#include "VoxelVolume.h"



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

OctreeVolumeBranch::~OctreeVolumeBranch()
{
	for (OctreeVolumeNode* child : children)
		if (child != nullptr)
			delete child;
}

void OctreeVolumeBranch::Set(uint32 x, uint32 y, uint32 z, float value)
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
		(isFront	? 4 : 0) +
		(isTop		? 2 : 0) +
		(isRight	? 1 : 0);
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
			{
				node = new OctreeVolumeLeaf(this);
				((OctreeVolumeLeaf*)node)->Init();
			}
			else
				node = new OctreeVolumeBranch(this);
		}
	}


	// Adjust x,y,z for child node
	node->Set(
		isRight ? x - halfRes : x,
		isTop	? y - halfRes : y,
		isFront ? z -halfRes: z,
		value
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


///
/// Leaf
///

OctreeVolumeLeaf::OctreeVolumeLeaf(OctreeVolumeNode* parent) : OctreeVolumeNode(parent), m_value(DEFAULT_VALUE)
{
}
OctreeVolumeLeaf::~OctreeVolumeLeaf() 
{
}

void OctreeVolumeLeaf::Init() 
{
	m_coordinates = GetParentBranch()->FetchRootCoords(this);
}


///
/// Combined octree
///

VolumeOctree::VolumeOctree(uint16 resolution)
{
	if (resolution == 1)
		resolution = 2;
	
	m_root = new OctreeVolumeBranch(resolution);
}
VolumeOctree::~VolumeOctree()
{
	delete m_root;
}