#pragma once
#include "Object.h"
#include "VoxelVolume.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "MarchingCubes.h"

#include <unordered_map>
#include <array>


class Material;
class LayeredVolume;
class OctreeLayer;


/**
* An octree node to be used in layers
*/
class OctreeLayerNode 
{
	///
	/// Vars
	///
private:
	const uint32 m_id;
	uint8 m_caseIndex = 0;
	uint8 m_childFlags = 0;

	std::array<float, 8> m_values;
	float m_average;
	float m_stdDeviation;
	OctreeLayer* m_layer;

public:
	OctreeLayerNode(const uint32& id, OctreeLayer* layer, const bool& initaliseValues = true);

	/**
	* Called just before this node is going to be deleted (During runtime)
	* This won't get called at the end cleanup
	*/
	void OnSafeDestroy();
	
	/**
	* Notify this node of a change in value
	* @param corner			The corner id to store this value in 
	* @param volume			The volume this value has originated from
	* @param value			The new value 
	*
	*/
	void Push(const uint32& corner, const LayeredVolume* volume, const float& value);
	
	/**
	* Build the actual mesh for just this node
	* @param isoLevel			The iso level to build at
	* @param builder			The builder which will create this mesh
	* @param highestLayer		The highest layer that is being built
	* @param maxDepthOffset		How much deeped should be considered for meshing
	*/
	void BuildMesh(const float& isoLevel, MeshBuilderMinimal& builder, OctreeLayer* highestLayer, const uint32& maxDepthOffset);

	///
	/// Tree Funcs
	///
public:
	#define CHILD_OFFSET_BK_BOT_L 0
	#define CHILD_OFFSET_BK_BOT_R 1
	#define CHILD_OFFSET_BK_TOP_L 2
	#define CHILD_OFFSET_BK_TOP_R 3
	#define CHILD_OFFSET_FR_BOT_L 4
	#define CHILD_OFFSET_FR_BOT_R 5
	#define CHILD_OFFSET_FR_TOP_L 6
	#define CHILD_OFFSET_FR_TOP_R 7

	/**
	* Retreive the parent for this node
	* @returns The id of the parent
	*/
	uint32 GetParentID() const;

	/**
	* Retreive a children's id for this node
	* Child Offset Offset:
	* 0: Back Bottom Left
	* 1: Back Bottom Right
	* 2: Back Top Left
	* 3: Back Top Right
	* 4: Front Bottom Left
	* 5: Front Bottom Right
	* 6: Front Top Left
	* 7: Front Top Right
	* @param offset			The child's offset you're trying to receive
	* @returns The actual id of the child
	*/
	uint32 GetChildID(const uint32& offset) const;

	/**
	* The offset that this node is at as a child for its parent
	*/
	uint32 GetOffsetAsChild() const;

	/**
	* Retreive all of the children for this node
	* @param outList		Where to store the children
	*/
	void FetchChildren(std::array<OctreeLayerNode*, 8>& outList) const;


	///
	/// Merging functions
	///
public:
	/**
	* Does the case of this node only contain a single connected face
	* @returns True if only a single face exist for this case
	*/
	bool IsSimpleCase() const;

	/**
	* Is it safe to merge this nodes children
	* @param maxDepthOffset		How much deeped should be considered for meshing
	* @returns True if this node can be safely rendered at its own resolution
	*/
	bool RequiresHigherDetail(const uint32& maxDepthOffset) const;

private:
	/**
	* Recalculate stats relevant to this node
	*/
	void RecalculateStats();

	/**
	* Count how many intersections exist for this edge
	* (Will check all children down to leafs)
	* @param edge			The MC id of the edge
	* @param max			If the count exceeds this value, the calls will stop
	* @param runningCount	Passed from highest level in call stack to prevent calls going on for too long
	* @returns True if there are multiple intersections
	*/
	uint32 IntersectionCount(const uint32& edge, const uint32& max = 0, uint32* runningCount = nullptr) const;

private:
	/**
	* Update the child existence flag for the given child
	* @param offset				The offset of the child
	* @param exists				Does this child node exist
	*/
	inline void SetChildFlag(const uint32& offset, const bool& exists) 
	{ 
		const uint32 flag = (1 << offset);
		if (exists) m_childFlags |= flag;
		else m_childFlags &= ~flag;
	}

	/**
	* Check whether a child exists or not (According to this node)
	* @param offset				The offset of the child
	* @returns True if the child exists
	*/
	inline bool GetChildFlag(const uint32& offset) const
	{
		const uint32 flag = (1 << offset);
		return (m_childFlags & flag) != 0;
	}

	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_id; }
	inline bool FlaggedForDeletion() const { return m_caseIndex == 0 && m_childFlags == 0; }

	inline uint8 GetCaseIndex() const { return m_caseIndex; }
	inline bool HasEdge(const uint32& edgeId) const { return (MC::CaseRequiredEdges[m_caseIndex] & edgeId) != 0; }

private:
	inline uint32 GetIndex(const uint32& x, const uint32& y, const uint32& z) const { return x + 2 * (y + 2 * z); }
};


/**
* A specific layer in the octree
*/
class OctreeLayer 
{
	///
	/// Vars
	///
private:
	std::unordered_map<uint32, OctreeLayerNode*> m_nodes;
	const uint32 m_nodeResolution;
	const uint32 m_layerResolution;
	const uint32 m_depth;
	const uint32 m_startIndex;	// The index at which this layer will start it's nodes
	const uint32 m_endIndex;	// The index at which this layer will start it's nodes
	LayeredVolume* m_volume;
public:
	OctreeLayer* previousLayer = nullptr;
	OctreeLayer* nextLayer = nullptr;

public:
	OctreeLayer(LayeredVolume* volume, const uint32& depth, const uint32& height, const uint32& nodeRes);
	~OctreeLayer();

	/**
	* Handle when a value has been changed in the volume
	* @param x,y,z				The world coordinate
	* @param value				The new value for this coord
	* @returns If any changes have occured to this layer during this push
	*/
	bool HandlePush(const uint32& x, const uint32& y, const uint32& z, const float& value);

	/**
	* Build a mesh starting at this layer
	* @param maxDepthOffset		How much deeped should be considered for meshing
	* @param builder		The builder which will create this mesh
	*/
	void BuildMesh(MeshBuilderMinimal& builder, const uint32& maxDepthOffset);

	/**
	* Should the edge connecting these 2 points be overridden (i.e. is a high-res edge meeting a low-res edge)
	* @param a,b				The desired edge to build (Only 1 axis is expected to change value in this pair)
	* @param maxDepthOffset		How much deeped should be considered for meshing
	* @param overrideOutput		Where to store the new edge, if overriden
	* @returns True if this edge has been overridden
	*/
	bool OverrideEdge(const uvec3& a, const uvec3& b, const uint32& maxDepthOffset, vec3& overrideOutput) const;
private:
	/**
	* Project a high-res point onto a low-res edge
	* @param a,b				The desired edge to build (Only 1 axis is expected to change value in this pair)
	* @param maxDepthOffset		How much deeped should be considered for meshing
	* @param overrideOutput		Where to store the new edge, if overriden
	* @param c00-c11			The corners of the face to project onto
	* @returns True if this edge has been overridden
	*/
	bool ProjectEdgeOntoFace(const uvec3& a, const uvec3& b, const uint32& maxDepthOffset, vec3& overrideOutput, const uvec3& c00, const uvec3& c01, const uvec3& c10, const uvec3& c11) const;

private:
	/**
	* Push a specific value onto a specific node
	* @param localCoords		The local coordinates of the value
	* @param offset				The offset of the nodes to inform (Expecting the node has this value as one of it's corners)
	* @param value				The actual value to push
	*/
	void PushValueOntoNode(const uvec3& localCoords, const ivec3& offset, const float& value);

	/**
	* Attempt to retrieve a node based on a position
	* @param localCoords		The local coordinates of the value
	* @param offset				The offset of the nodes to inform
	* @param outNode			Where to store the node, if found
	*/
	bool AttemptNodeOffsetFetch(const uvec3& localCoords, const ivec3& offset, OctreeLayerNode*& outNode) const;

public:
	/**
	* Retreive the ID for this local coordinate
	* @param x,y,z				The local coordinate
	* @returns The node ID to be used
	*/
	inline uint32 GetID(const uint32& x, const uint32& y, const uint32& z) const 
	{ 
		const uint32 width = m_layerResolution - 1;
		return m_startIndex + x + width *(y + width * z);
	}

	/**
	* Retreive the local coordinates from an id 
	* @param id					The id of the node (Expected to in this layer)
	*/
	inline uvec3 GetLocalCoords(const uint32& id) const
	{
		const uint32 localId = id - m_startIndex;
		const uint32 width = m_layerResolution - 1;
		
		return uvec3(
			localId % width,
			(localId / width) % width,
			localId / (width * width)
		);
	}

	/**
	* Attempt to fetch the node with this id in this layer or above/below
	* @param id					The id of the node to hunt for
	* @param outNode			Where to store the node, if found
	* @param createIfAbsent		If the node is not found (But in a valid layer) should it be created
	* @returns If the node was sucesfully found
	*/
	bool AttemptNodeFetch(const uint32& id, OctreeLayerNode*& outNode, const bool& createIfAbsent);

	///
	/// Getters & Setters
	///
public:
	inline uint32 GetNodeResolution() const { return m_nodeResolution; }
	inline uint32 GetLayerResolution() const { return m_layerResolution; }
	inline uint32 GetStride() const { return m_nodeResolution - 1; }
	inline uint32 GetDepth() const { return m_depth; }

	inline uint32 GetStartID() const { return m_startIndex; }
	inline uint32 GetEndID() const { return m_endIndex; }

	inline LayeredVolume* GetVolume() const { return m_volume; }
};


/**
* Object for holding an octree-layered volume
*/
class LayeredVolume : public Object, public IVoxelVolume
{
private:
	///
	/// Rendering Vars
	///
	Material* m_material = nullptr;
	Material* m_wireMaterial = nullptr;
	Material* m_debugMaterial = nullptr;

	///
	/// Volume vars
	///
	std::vector<float> m_data;
	std::vector<OctreeLayer*> m_layers; // Declared in order from least depth to greatest
	float m_isoLevel;
	vec3 m_scale = vec3(1, 1, 1);
	uvec3 m_resolution;
	uint32 m_octreeRes;

	class Mesh* TEST_MESH;
	bool TEST_REBUILD;

public:
	LayeredVolume();
	virtual ~LayeredVolume();

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

	///
	/// Getters & Setters
	///
private:
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + m_resolution.x * (y + m_resolution.y * z); }
public:
	inline uint32 GetOctreeResolution() const { return m_octreeRes; }
};

