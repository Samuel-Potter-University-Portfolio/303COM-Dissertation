#pragma once
#include "Object.h"
#include "Camera.h"
#include "VoxelVolume.h"


enum class InteractionShape 
{
	Point = 0,
	Sphere,
	Cube,
	Unknown
};



class SpectatorController : public Object
{
private:
	///
	/// General Vars
	///
	Camera* camera;
	uint32 m_shapeSize = 10;
	InteractionShape m_interactionShape;
	
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

