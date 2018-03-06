#pragma once
#include "Object.h"
#include "Mesh.h"
#include "Material.h"
#include "VolumeData.h"


class VoxelVolume : public Object
{
private:
	///
	/// General Vars
	///
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
	VolumeData m_data;

public:
	VoxelVolume();
	virtual ~VoxelVolume();

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;
	virtual void Draw(const class Window* window, const float& deltaTime) override;


	void BuildMesh();
};

