#pragma once
#include "Object.h"
#include "VoxelVolume.h"
#include "MeshBuilder.h"
#include "Mesh.h"

#include <unordered_map>
#include <array>


class Material;
class LayeredVolume;


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
	std::array<float, 8> m_values;

public:
	OctreeLayerNode(const uint32& id) : m_id(id), m_values({ DEFAULT_VALUE }) {}
	
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
	* @param isoLevel		The iso level to build at
	* @param layer			The layer tha his node is a member of 
	* @param builder		The builder which will create this mesh
	*/
	void BuildMesh(const float& isoLevel, class OctreeLayer* layer, MeshBuilderMinimal& builder);


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

#define CHILD_OFFSET_BACK_BOTTOM_LEFT 0
#define CHILD_OFFSET_BACK_BOTTOM_RIGHT 1
#define CHILD_OFFSET_BACK_TOP_LEFT 2
#define CHILD_OFFSET_BACK_TOP_RIGHT 3
#define CHILD_OFFSET_FRONT_BOTTOM_LEFT 4
#define CHILD_OFFSET_FRONT_BOTTOM_RIGHT 5
#define CHILD_OFFSET_FRONT_TOP_LEFT 6
#define CHILD_OFFSET_FRONT_TOP_RIGHT 7
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
	inline uint32 GetChildID(const uint32& offset) { return m_id * 8 + offset; }

	inline bool FlaggedForDeletion() const { return m_caseIndex == 0; }

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
	const uint32 m_resolution;
	const uint32 m_effectiveTotalResolution;
	const uint32 m_depth;
	const uint32 m_startIndex;	// The index at which this layer will start it's nodes
	LayeredVolume* m_volume;


public:
	OctreeLayer(LayeredVolume* volume, const uint32& depth, const uint32& height, const uint32& resolution);
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
	inline uint32 GetID(const uint32& x, const uint32& y, const uint32& z) const { return m_startIndex + x + m_effectiveTotalResolution *(y + m_effectiveTotalResolution * z); }

	/**
	* Retreive the local coordinates from an id 
	* @param id					The id of the node (Expected to in this layer)
	*/
	inline uvec3 GetLocalCoords(const uint32& id) const
	{
		const uint32 localId = id - m_startIndex;

		return uvec3(
			localId % m_effectiveTotalResolution,
			(localId / m_effectiveTotalResolution) % m_effectiveTotalResolution,
			localId / (m_effectiveTotalResolution * m_effectiveTotalResolution)
		);
	}

	///
	/// Getters & Setters
	///
public:
	inline uint32 GetResolution() const { return m_resolution; }
	inline uint32 GetStride() const { return m_resolution - 1; }
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
	std::vector<OctreeLayer> m_layers; // Declared in order from least depth to greatest
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

