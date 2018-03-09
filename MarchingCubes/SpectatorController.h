#pragma once
#include "Object.h"
#include "Camera.h"
#include "VoxelVolume.h"


class SpectatorController : public Object
{
private:
	///
	/// General Vars
	///
	Camera* camera;
	
	///
	/// Lookat Vars
	///
	bool bLookingAtVoxel;
	VoxelHitInfo lookatVoxel;
	class Mesh* m_mesh = nullptr;
	class Material* m_material = nullptr;


public:
	virtual ~SpectatorController();

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;

	virtual void Draw(const Window* window, const float& deltaTime);
};

