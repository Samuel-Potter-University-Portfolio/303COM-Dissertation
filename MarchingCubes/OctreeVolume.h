#pragma once
#include "Object.h"

#include "Mesh.h"
#include "MeshBuilder.h"
#include "Material.h"
#include "VoxelVolume.h"


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

	/**
	* Called safely after this has been added to the tree
	*/
	virtual void Init() = 0;

	/**
	* Set the value for this coord
	* @param x,y,z				The tree coords to set
	* @param value				The desired value
	* @param additions			An optional array to access any newly added nodes
	*/
	virtual void Set(uint32 x, uint32 y, uint32 z, float value, std::vector<OctreeVolumeNode*>* additions = nullptr) = 0;

	/**
	* Get the value for this coord
	* @param x,y,z				The tree coords to get
	* @returns The value at this node or DEFAULT_VALUE if no node exits
	*/
	virtual float Get(uint32 x, uint32 y, uint32 z) const = 0;

	/// Calculate the average weight at this node
	virtual float GetValueAverage() const = 0;
	/// Calulate the standard deviation of the children of this node
	virtual float GetValueDeviation() const = 0;


	/**
	* Constructs a mesh to view the octree
	* @param build				The builder to upload data to
	* @param isoLevel			The isolevel cut-off to use for MC
	*/
	virtual void ConstructDebugMesh(MeshBuilderMinimal& build, float isoLevel) = 0;

	/**
	* Attempt to construct a mesh for this node
	* @param build				The builder to upload data to
	* @param isoLevel			The isolevel cut-off to use for MC
	* @param depthDeltaAcc		How much higher than this node can be considered
	* @param depthDeltaDec		How much lower than this node can be considered
	*/
	virtual void ConstructMesh(MeshBuilderMinimal& build, float isoLevel, int32 depthDeltaAcc, int32 depthDeltaDec) = 0;




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
	uvec3 m_coordinates;
	float m_valueAverage;
	float m_valueDeviation;

public:
	OctreeVolumeBranch(uint16 resolution);
	OctreeVolumeBranch(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeBranch();

	virtual void Init() override;

	virtual void Set(uint32 x, uint32 y, uint32 z, float value, std::vector<OctreeVolumeNode*>* additions = nullptr) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) const override;

	virtual float GetValueAverage() const override { return m_valueAverage; }
	virtual float GetValueDeviation() const override { return m_valueDeviation; }

	virtual void ConstructDebugMesh(MeshBuilderMinimal& build, float isoLevel) override;
	virtual void ConstructMesh(MeshBuilderMinimal& build, float isoLevel, int32 depthDeltaAcc, int32 depthDeltaDec) override;

	/**
	* Fetch the root coordinates for this child
	* @param child			A node which is expected to be a child of this node
	* @returns The coordinates (In relation to the root) for the child
	*/
	uvec3 FetchRootCoords(const OctreeVolumeNode* child) const;

	/**
	* Fetch the value to during building for this coordinate
	* @param source			The child who is requesting the value
	* @param maxDepth		The max depth to look at
	* @param x,y,z			The relative coordinates (To the child) to fetch the value for
	* @returns The value to use during building
	*/
	float FetchBuildIsolevel(const OctreeVolumeNode* source, int32 maxDepth, int32 x, int32 y, int32 z) const;


	/**
	* Fetch the value to during building for this coordinate
	* @param maxDepth		The max depth to look at
	* @param x,y,z			The coordinates to fetch the value for (Assuming they're withing correct range)
	* @returns The value to use during building
	*/
	float FetchBuildIsolevel(int32 maxDepth, int32 x, int32 y, int32 z) const;


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
	VoxelPartialMeshData* m_meshData = nullptr;

public:
	OctreeVolumeLeaf(OctreeVolumeNode* parent);
	virtual ~OctreeVolumeLeaf();

	virtual void Init() override;

	virtual void Set(uint32 x, uint32 y, uint32 z, float value, std::vector<OctreeVolumeNode*>* additions = nullptr) override;
	virtual float Get(uint32 x, uint32 y, uint32 z) const override { return m_value; }

	virtual float GetValueAverage() const override { return m_value; }
	virtual float GetValueDeviation() const override { return 0.0f; }

	virtual void ConstructDebugMesh(MeshBuilderMinimal& build, float isoLevel) override;
	virtual void ConstructMesh(MeshBuilderMinimal& build, float isoLevel, int32 depthDeltaAcc, int32 depthDeltaDec) override;

protected:
	inline OctreeVolumeBranch* GetParentBranch() const { return (OctreeVolumeBranch*)GetParent(); }
};



/**
* The Octree based volume which is the final result of my dissertation
*/
class OctreeVolume : public Object, public IVoxelVolume
{
private:
	///
	/// Rendering Vars
	///
	Material* m_material = nullptr;
	Material* m_wireMaterial = nullptr;

	///
	/// Volume vars
	///
	OctreeVolumeNode* m_root;
	float m_isoLevel;
	vec3 m_scale = vec3(1, 1, 1);
	uvec3 m_resolution;


	/// TEST VARS TODO REMOVE
	std::vector<OctreeVolumeNode*> testLayer;
	Mesh* m_mesh;
	Mesh* TEST_MESH;
	bool TEST_REBUILD = false;

public:
	OctreeVolume();
	virtual ~OctreeVolume();


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
	virtual float Get(uint32 x, uint32 y, uint32 z) override;

	virtual uvec3 GetResolution() const override { return m_resolution; }
	virtual bool SupportsDynamicResolution() const override { return false; }

	virtual float GetIsoLevel() const override { return m_isoLevel; }

	/// TODO MAKE PROPER
private:
	void BuildMesh();
	void BuildTreeMesh();

	///
	/// Getters & Setters
	///
private:
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + m_resolution.x * (y + m_resolution.y * z); }
public:
	inline vec3 GetScale() const { return m_scale; }
};

