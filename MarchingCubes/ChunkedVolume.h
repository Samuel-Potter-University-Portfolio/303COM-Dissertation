#pragma once
#include "Object.h"

#include "Mesh.h"
#include "Material.h"
#include "VoxelVolume.h"

#include <vector>



/**
* Represents the data for a single chunk of data
*/
struct VoxelChunk
{
public:
	///
	/// Rendering vars
	///
	Mesh* mesh = nullptr;
	bool bRequiresRebuild;

private:
	///
	/// Volume vars
	///
	uvec3 m_offset;
	uint32 m_resolution;
	class ChunkedVolume* m_parent;

public:
	VoxelChunk(const uvec3& offset, uint32 resolution, ChunkedVolume* parent);
	~VoxelChunk();


	// TODO - MAKE PROPER
	void BuildMesh();
};



/**
* The chunked implementation for MC
*/
class ChunkedVolume : public Object, public IVoxelVolume
{
private:
	///
	/// Rendering Vars
	///
	Material* m_material = nullptr;

	///
	/// Volume vars
	///
	std::vector<VoxelChunk*> m_chunks;
	const uint32 chunkSize;
	float* m_data = nullptr;
	float m_isoLevel;
	vec3 m_scale = vec3(1, 1, 1);
	uvec3 m_resolution;

public:
	ChunkedVolume();
	virtual ~ChunkedVolume();


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

	///
	/// Getters & Setters
	///
private:
	/** Get the index for a voxel in data */
	inline uint32 GetVoxelIndex(uint32 x, uint32 y, uint32 z) const { return x + m_resolution.x * (y + m_resolution.y * z); }

	/**
	* Get the internal chunk coordniate for this
	* @param x,y,z The object-space x,y,z coordinates for a voxel
	*/
	inline uvec3 GetChunkCoords(uint32 x, uint32 y, uint32 z) const
	{
		return uvec3(
			x / chunkSize,
			y / chunkSize,
			z / chunkSize
		);
	}
	/**
	* Get the internal index for this chunk 
	* @param x,y,z The object-space x,y,z coordinates for a voxel
	*/
	inline uint32 GetChunkIndex(uint32 x, uint32 y, uint32 z) const 
	{ 
		uvec3 coord = GetChunkCoords(x, y, z);
		return coord.x + (m_resolution.x/chunkSize) * (coord.y + (m_resolution.y / chunkSize) * coord.z);
	}
};

