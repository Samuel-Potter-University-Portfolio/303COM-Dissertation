#include "VolumeOctree.h"
#include "Logger.h"



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
	if (resolution <= NODE_STRIDE)
		LOG_ERROR("Branch node made with size(%i) <= Node Stride(%i)", GetResolution(), NODE_STRIDE);
}

OctreeVolumeBranch::OctreeVolumeBranch(OctreeVolumeNode* parent) : OctreeVolumeNode(parent)
{
	if (GetResolution() <= NODE_STRIDE)
		LOG_ERROR("Branch node made with size(%i) <= Node Stride(%i)", GetResolution(), NODE_STRIDE);
}

OctreeVolumeBranch::~OctreeVolumeBranch()
{
	for (OctreeVolumeNode* child : children)
		if (child != nullptr)
			delete child;
}

void OctreeVolumeBranch::Set(uint32 x, uint32 y, uint32 z, float value)
{
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
			if (halfRes > NODE_STRIDE)
				node = new OctreeVolumeBranch(this);
			else
				node = new OctreeVolumeLeaf(this);
		}
	}


	// Adjust x,y,z for child node
	node->Set(
		isRight ? x - halfRes : x,
		isTop	? y - halfRes : y,
		isFront ? z -halfRes: z,
		value
	);
}

float OctreeVolumeBranch::Get(uint32 x, uint32 y, uint32 z) const
{
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



///
/// Leaf
///

OctreeVolumeLeaf::OctreeVolumeLeaf(OctreeVolumeNode* parent) : OctreeVolumeNode(parent)
{
	m_data = new float[GetResolution() * GetResolution() * GetResolution()]{ DEFAULT_VALUE };
}
OctreeVolumeLeaf::~OctreeVolumeLeaf() 
{
	delete[] m_data;
}

void OctreeVolumeLeaf::Set(uint32 x, uint32 y, uint32 z, float value) 
{
#ifdef _DEBUG
	uint32 index = GetIndex(x, y, z);
	if (index < 0 || index >= GetResolution() * GetResolution() * GetResolution())
	{
		LOG_ERROR("Set Index out of range: %i", index);
		return;
	}
#endif
	m_data[GetIndex(x, y, z)] = value;
}
float OctreeVolumeLeaf::Get(uint32 x, uint32 y, uint32 z) const
{
#ifdef _DEBUG
	uint32 index = GetIndex(x, y, z);
	if (index < 0 || index >= GetResolution() * GetResolution() * GetResolution())
	{
		LOG_ERROR("Set Index out of range: %i", index);
		return DEFAULT_VALUE;
	}
#endif
	return m_data[GetIndex(x, y, z)];
}



///
/// Combined octree
///

VolumeOctree::VolumeOctree(uint16 resolution)
{
	if (resolution > NODE_STRIDE)
		m_root = new OctreeVolumeBranch(resolution);
	else
		m_root = new OctreeVolumeLeaf(nullptr);
}
VolumeOctree::~VolumeOctree()
{
}