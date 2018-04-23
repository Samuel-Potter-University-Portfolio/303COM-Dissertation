#pragma once
#include "Object.h"
#include "VoxelVolume.h"

#include "Material.h"
#include "Mesh.h"

#include <map>
#include <unordered_map>
#include <array>
#include <functional>


class OctRepNode;

/**
* Packet to return any nodes which have changed
*/
struct OctRepNotifyPacket 
{
	std::vector<OctRepNode*> newNodes;
	std::vector<OctRepNode*> updatedNodes;
	std::vector<OctRepNode*> deletedNodes;
};



/**
* Octree node which stores the representation of the data rather than the actual data itself
* Each node stores enough data to work out it's own model without having to know other's info
*/
class OctRepNode 
{
private:
	///
	/// General Vars
	///
	OctRepNode const* m_parent;
	const uint8 m_depth;
	const uint16 m_resolution;
	std::array<OctRepNode*, 8> m_children{ nullptr };

	///
	/// Volume/Drawing Vars
	///
	const uvec3 m_offset;
	std::array<float, 8> m_values{ DEFAULT_VALUE };
	float m_stdDeviation;
	float m_average;

public:
	OctRepNode(uint16 resolution);
	OctRepNode(OctRepNode* parent, const uvec3& offset);
	~OctRepNode();

	/**
	* Push a (new or changed) value into this tree
	* @param x,y,z				The coordinates local to this node
	* @param value				The change in value
	* @param outPacket			Information about the tree which has changed
	*/
	void Push(const uint32& x, const uint32& y, const uint32& z, const float& value, OctRepNotifyPacket& outPacket);

	/**
	* Recaluclate cached stats e.g. stdDeviation, average
	*/
	void RecalculateStats();


	typedef std::function<vec3(const float& isoLevel, const uvec3& a, const uvec3& b, const uint32& minRes)> RetrieveEdgeCallback;
	/**
	* Generate the partial mesh for this node
	* @param isoLevel			The isoLevel to use in MC
	* @param edgeCallback		Retreive an appropriately placed edge
	* @param target				Where to store the newly generated mesh data
	*/
	void GeneratePartialMesh(const float& isoLevel, RetrieveEdgeCallback edgeCallback, VoxelPartialMeshData* target, const uint32& minRes);

	/**
	* Generate the partial mesh for this node
	* @param isoLevel			The isoLevel to use in MC
	* @param target				Where to store the newly generated mesh data
	*/
	void GenerateDebugPartialMesh(const float& isoLevel, VoxelPartialMeshData* target, const uint32& minRes);

	bool RequiresHigherDetail(const float& isoLevel, const uint32& minRes) const;

	bool HasMultipleEdgeIntersections(const float& isoLevel, const uint32& minRes) const;

	///
	/// Getters & Setters
	///
public:
	bool IsDefaultValues() const;

	inline float GetAverage() const { return m_average; }
	inline float GetStdDeviation() const { return m_stdDeviation; }

	inline bool IsRoot() const { return m_parent == nullptr; }
	inline bool IsLeaf() const { return m_resolution <= 2; }
	inline bool IsBranch() const { return m_resolution > 2; }

	inline const uvec3& GetOffset() const { return m_offset; }
	inline uint32 GetResolution() const { return m_resolution; }
	inline uint32 GetDepth() const { return m_depth; }
	uint32 GetChildResolution() const;

private:
	inline uint32 GetIndex(const uint32& x, const uint32& y, const uint32& z) const { return x + 2 * (y + 2 * z); }
};



/**
* Represents a single layer of an octree
*/
class OctreeRepLayer 
{
	///
	/// General Vars
	///
private:
	uint32 m_resolution; // The resolution this layer represents
	std::unordered_map<uvec3, OctRepNode*, uvec3_KeyFuncs> m_nodes; // All nodes which are in this layer
	const class OctreeRepVolume* m_volume;

public:
	OctreeRepLayer* nextLayer = nullptr;

public:
	OctreeRepLayer(const OctreeRepVolume* volume, const uint32& resolution) : m_volume(volume), m_resolution(resolution) {}

	/**
	* Calculate the edge from 2 given points
	* @param isoLevel		The isoLevel to place the edge at
	* @param a				Coord of first point
	* @param b				Coord of second point
	* @param minRes			The minimum resolution to consider before using default value
	*/
	vec3 RetrieveEdge(const float& isoLevel, const uvec3& a, const uvec3& b, const uint32& minRes);
	
	/**
	* Attempt to find a node if it exists
	* @param x,y,z			World coordinates
	* @param outNode		Where to store the retrieved node
	* @returns If the node was found
	*/
	bool AttemptGet(const uint32& x, const uint32& y, const uint32& z, OctRepNode*& outNode) const;


	/// Callback for when a new node is created
	void OnNewNode(OctRepNode* node);

	/// Callback for when a node is modified
	void OnModifyNode(OctRepNode* node);
	
	/// Callback for when a node deleted
	void OnDeleteNode(const OctRepNode* node);

private:
	vec3 ProjectCorners(const float& isoLevel, const uvec3& a, const uvec3& b, const uvec3& bottomLeft, const uvec3& bottomRight, const uvec3& topLeft, const uvec3& topRight) const;

public:
	inline uint32 GetResolution() const { return m_resolution; }
};



/**
* Represents an octree based volume, but where the data is still centralised
* The octree is used to more easily propagate rebuild/lod commands
*
* Stores nodes in layers, for easy querying
*/
class OctreeRepVolume : public Object, public IVoxelVolume
{
private:
	///
	/// Rendering Vars
	///
	Material* m_material = nullptr;
	Material* m_wireMaterial = nullptr;
	Material* m_debugMaterial = nullptr;
	Mesh* m_mesh = nullptr;
	Mesh* m_debugMesh = nullptr;

	///
	/// Volume vars
	///
	std::vector<float> m_data;
	OctRepNode* m_octree;
	std::vector<OctreeRepLayer> m_layers;

	float m_isoLevel;
	vec3 m_scale = vec3(1, 1, 1);
	uvec3 m_resolution;

	///
	/// Levels vars
	///
	std::unordered_map<OctRepNode*, VoxelPartialMeshData> m_nodeLevel;
	std::unordered_map<OctRepNode*, VoxelPartialMeshData> m_nodedebugLevel;


	bool TEST_REBUILD = false;

public:
	OctreeRepVolume();
	virtual ~OctreeRepVolume();
	///
	/// Object functions
	///
public:
	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;
	virtual void Draw(const class Window* window, const float& deltaTime) override;


	///
	/// Voxel Data functions
	///
public:
	virtual void Init(const uvec3& resolution, const vec3& scale) override;

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) override; // TODO - FIX THAT GET
	float Get(uint32 x, uint32 y, uint32 z) const;

	virtual uvec3 GetResolution() const override { return m_resolution; }
	virtual bool SupportsDynamicResolution() const override { return false; }

	virtual float GetIsoLevel() const override { return m_isoLevel; }


	// TODO - MAKE PROPER
	void BuildMesh();

	///
	/// Getters & Setters
	///
private:
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + m_resolution.x * (y + m_resolution.y * z); }
public:
	inline vec3 GetScale() const { return m_scale; }
};

