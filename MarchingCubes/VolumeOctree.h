#pragma once
#include "Common.h"

#ifndef DEFAULT_VALUE
#define DEFAULT_VALUE 0.0f
#endif

#ifndef NODE_STRIDE
#define NODE_STRIDE 4
#endif


/**
* Represents a single node in an octree
*/
class OctreeVolumeNode
{
private:
	///
	/// General Vars
	///
	OctreeVolumeNode* m_parent;
	uint16 m_depth;
	uint16 m_resolution;

public:
	OctreeVolumeNode(uint16 resolution);
	OctreeVolumeNode(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeNode();

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) = 0;
	virtual float Get(uint32 x, uint32 y, uint32 z) const = 0;

	///
	/// Getters & Setters
	///
public:
	inline OctreeVolumeNode* GetParent() const { return m_parent; }
	inline const uint16& GetDepth() const { return m_depth; }
	inline const uint16& GetResolution() const { return m_resolution; }
};



/**
* Represents a regular node which 
*/
class OctreeVolumeBranch : public OctreeVolumeNode
{
public:
	///
	/// All the chilren that this node can have
	///
	OctreeVolumeNode* children[8]{ nullptr };
	OctreeVolumeNode*& c000 = children[0]; // back bottom left
	OctreeVolumeNode*& c001 = children[1]; // back bottom right
	OctreeVolumeNode*& c010 = children[2]; // back top left
	OctreeVolumeNode*& c011 = children[3]; // back top right
	OctreeVolumeNode*& c100 = children[4]; // front bottom left
	OctreeVolumeNode*& c101 = children[5]; // front bottom right
	OctreeVolumeNode*& c110 = children[6]; // front top left
	OctreeVolumeNode*& c111 = children[7]; // front top right

public:
	OctreeVolumeBranch(uint16 resolution);
	OctreeVolumeBranch(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeBranch();

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) const override;
};



/**
* Represents the end node of an octree branch
*/
class OctreeVolumeLeaf : public OctreeVolumeNode
{
private:
	///
	/// General Vars
	///
	float* m_data = nullptr;

public:
	OctreeVolumeLeaf(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeLeaf();

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) const override;

private:
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + GetResolution() * (y + GetResolution() * z); }
};



/**
* Represents an Octree volume
*/
class VolumeOctree
{
private:
	///
	/// General Vars
	///
	OctreeVolumeNode* m_root;

public:
	VolumeOctree(uint16 resolution);
	~VolumeOctree();

	inline void Set(uint32 x, uint32 y, uint32 z, float value) { m_root->Set(x, y, z, value); }
	inline float Get(uint32 x, uint32 y, uint32 z) const { return m_root->Get(x, y, z); }

	inline OctreeVolumeNode* GetRootNode() const { return m_root; }
};