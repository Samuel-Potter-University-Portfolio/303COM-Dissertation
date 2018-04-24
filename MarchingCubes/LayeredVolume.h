#pragma once
#include "Object.h"
#include "VoxelVolume.h"
#include "MeshBuilder.h"
#include "Mesh.h"

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
	OctreeLayer* m_layer;
	const uint32 m_id;
	uint8 m_caseIndex = 0;
	uint8 m_childFlags = 0;
	std::array<float, 8> m_values;

public:
	OctreeLayerNode(const uint32& id, OctreeLayer* layer) : m_id(id), m_values({ DEFAULT_VALUE }), m_layer(layer) {}
	
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
	* @param maxDepthOffset		How much deeped should be considered for meshing
	*/
	void BuildMesh(const float& isoLevel, MeshBuilderMinimal& builder, const uint32& maxDepthOffset);


	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_id; }

	/**
	* Retreive the parent for this node
	* @returns The id of the parent
	*/
	inline uint32 GetParentID() const { return m_id / 8; }

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
	inline uint32 GetChildID(const uint32& offset) const { return m_id * 8 + offset; }
	#define CHILD_OFFSET_BK_BOT_L 0
	#define CHILD_OFFSET_BK_BOT_R 1
	#define CHILD_OFFSET_BK_TOP_L 2
	#define CHILD_OFFSET_BK_TOP_R 3
	#define CHILD_OFFSET_FR_BOT_L 4
	#define CHILD_OFFSET_FR_BOT_R 5
	#define CHILD_OFFSET_FR_TOP_L 6
	#define CHILD_OFFSET_FR_TOP_R 7

	/**
	* Retreive all of the children for this node
	* @param outList		Where to store the children
	*/
	void FetchChildren(std::array<OctreeLayerNode*, 8>& outList) const;

	/**
	* Does the case of this node only contain a single connected face
	* @returns True if only a single face exist for this case
	*/
	bool IsSimpleCase() const;

	/**
	* Is it safe to merge this nodes children
	* @returns True if this node can be safely rendered at its own resolution
	*/
	bool IsMergeSafe() const;

private:
	inline uint32 GetIndex(const uint32& x, const uint32& y, const uint32& z) const { return x + 2 * (y + 2 * z); }
public:
	inline bool FlaggedForDeletion() const { return m_caseIndex == 0 && m_childFlags == 0; }
	inline uint8 GetCaseIndex() const { return m_caseIndex; }
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
	* @param builder		The builder which will create this mesh
	*/
	void BuildMesh(MeshBuilderMinimal& builder);

private:
	/**
	* Push a specific value onto a specific node
	* @param localCoords		The local coordinates of the value
	* @param offset				The offset of the nodes to inform (Expecting the node has this value as one of it's corners)
	* @param value				The actual value to push
	*/
	void PushValueOntoNode(const uvec3& localCoords, const ivec3& offset, const float& value);

public:
	/**
	* Retreive the ID for this local coordinate
	* @param x,y,z				The local coordinate
	* @returns The node ID to be used
	*/
	inline uint32 GetID(const uint32& x, const uint32& y, const uint32& z) const { return m_startIndex + x + m_layerResolution *(y + m_layerResolution * z); }

	/**
	* Retreive the local coordinates from an id 
	* @param id					The id of the node (Expected to in this layer)
	*/
	inline uvec3 GetLocalCoords(const uint32& id) const
	{
		const uint32 localId = id - m_startIndex;

		return uvec3(
			localId % m_layerResolution,
			(localId / m_layerResolution) % m_layerResolution,
			localId / (m_layerResolution * m_layerResolution)
		);
	}

	/**
	* Attempt to fetch the node with this id in this layer or above/below
	* @param id				The id of the node to hunt for
	* @param outNode		Where to store the node, if found
	* @returns If the node was sucesfully found
	*/
	bool AttemptNodeFetch(const uint32& id, OctreeLayerNode*& outNode) const;

	///
	/// Getters & Setters
	///
public:
	inline uint32 GetNodeResolution() const { return m_nodeResolution; }
	inline uint32 GetLayerResolution() const { return m_layerResolution; }
	inline uint32 GetStride() const { return m_nodeResolution - 1; }
	inline uint32 GetDepth() const { return m_depth; }
	inline uint32 GetStartID() const { return m_startIndex; }

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

