#pragma once
#include "Common.h"
#include <vector>



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
	uint8 m_depth;
	uint16 m_resolution;

	 

public:
	OctreeVolumeNode(uint16 resolution);
	OctreeVolumeNode(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeNode();

	/// Set the value at this sub-coordinate
	virtual void Set(uint32 x, uint32 y, uint32 z, float value) = 0;
	/// Get the value at this sub-coordinate
	virtual float Get(uint32 x, uint32 y, uint32 z) const = 0;

	/// Calculate the average weight at this node
	virtual float GetValueAverage() const = 0;
	/// Calulate the standard deviation of the children of this node
	virtual float GetValueDeviation() const = 0;


	///
	/// Getters & Setters
	///
public:
	inline OctreeVolumeNode* GetParent() const { return m_parent; }
	inline const uint8& GetDepth() const { return m_depth; }
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

private:
	float m_valueAverage;
	float m_valueDeviation;

public:
	OctreeVolumeBranch(uint16 resolution);
	OctreeVolumeBranch(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeBranch();

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) const override;

	virtual float GetValueAverage() const override { return m_valueAverage; }
	virtual float GetValueDeviation() const override { return m_valueDeviation; }

	/**
	* Fetch the root coordinates for this child
	* @param child			A node which is expected to be a child of this node
	* @returns The coordinates (In relation to the root) for the child
	*/
	uvec3 FetchRootCoords(const OctreeVolumeNode* child) const;

protected:
	inline OctreeVolumeBranch* GetParentBranch() const { return (OctreeVolumeBranch*)GetParent(); }

private:
	/// Recalulate the average and deviation for this node
	void RecalculateStats();
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
	float m_value;
	uvec3 m_coordinates;

public:
	OctreeVolumeLeaf(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeLeaf();

	/** Called safely after this has been added to the tree */
	void Init();

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override { m_value = value; }
	virtual float Get(uint32 x, uint32 y, uint32 z) const override { return m_value; }

	virtual float GetValueAverage() const override { return m_value; }
	virtual float GetValueDeviation() const override { return 0.0f; }
protected:
	inline OctreeVolumeBranch* GetParentBranch() const { return (OctreeVolumeBranch*)GetParent(); }
};


/**
* Represents a layer of a specific level-of-detail
*/
struct OctreeDetailLevel
{
	struct NodeMesh
	{
		OctreeVolumeNode* node;
		class Mesh* mesh;		
	};

	std::vector<NodeMesh*> nodes;
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
	std::vector<OctreeDetailLevel> m_detailLevels;

public:
	VolumeOctree(uint16 resolution);
	~VolumeOctree();

	inline void Set(uint32 x, uint32 y, uint32 z, float value) { m_root->Set(x, y, z, value); }
	inline float Get(uint32 x, uint32 y, uint32 z) const { return m_root->Get(x, y, z); }

	inline OctreeVolumeNode* GetRootNode() const { return m_root; }
};