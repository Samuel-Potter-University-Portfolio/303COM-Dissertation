#pragma once
#include "Object.h"

#include "Mesh.h"
#include "Material.h"
#include "VoxelVolume.h"



/**
* The default (unoptimized) implementation for MC
*/
class DefaultVolume : public Object, public IVoxelVolume
{
private:
	///
	/// Rendering Vars
	///
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
	Material* m_wireMaterial = nullptr;
	bool bRequiresRebuild;

	///
	/// Volume Vars
	///
	float m_isoLevel;
	float* m_data = nullptr;
	vec3 m_scale = vec3(1, 1, 1);
	uvec3 m_resolution;

public:
	DefaultVolume();
	virtual ~DefaultVolume();

	///
	/// Object functions
	///
public:
	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;
	virtual void Draw(const class Window* window, const float& deltaTime) override;


	// TODO - MAKE PROPER
	void BuildMesh();


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
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + m_resolution.x * (y + m_resolution.y * z); }
public:
	inline vec3 GetScale() const { return m_scale; }
};

