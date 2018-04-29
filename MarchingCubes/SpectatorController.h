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

struct IsoChange 
{
	uvec3 coord;
	float value;
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
	class IVoxelVolume* currentVolume = nullptr;

	///
	/// Serialization
	///
	bool bIsRecording;
	bool bIsPlayback;
	std::vector<IsoChange> currentChanges;
	std::string demoFileName;
	uint32 playbackIndex;
	std::vector<std::vector<IsoChange>> playbackChanges;

public:
	virtual ~SpectatorController();

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;

	virtual void Draw(const Window* window, const float& deltaTime);

private:
	void Set(const uint32& x, const uint32& y, const uint32& z, const float& value);
};

