#pragma once
#include "Object.h"
#include "Ray.h"

#include "Mesh.h"
#include "Material.h"
#include "VolumeData.h"


struct VoxelHitInfo 
{
	float value;
	float surfaceValue;
	ivec3 coord;
	ivec3 surface;
};


class VoxelVolume : public Object
{
private:
	///
	/// General Vars
	///
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
	VolumeData m_data;
	float m_isoLevel;

public:
	VoxelVolume();
	virtual ~VoxelVolume();

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;
	virtual void Draw(const class Window* window, const float& deltaTime) override;

	/**
	* Fetch information about this voxel
	* @param ray				The ray to cast at the volume
	* @param outHit				Where to store the information about the voxel
	* @param maxDistance		The maximum distance of the ray
	*/
	virtual bool Raycast(const Ray& ray, VoxelHitInfo& outHit, float maxDistance = 100.0f);

	// TODO - MAKE PROPER
	inline void Set(uint32 x, uint32 y, uint32 z, float value) { m_data.Set(x, y, z, value); BuildMesh(); }

	void BuildMesh();
};

