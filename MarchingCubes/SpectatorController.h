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


struct VoxelFrame 
{
	std::vector<VoxelDelta> deltas;
	VoxelBuildResults results;

	inline void clear() 
	{
		deltas.clear();
		results.clear();
	}
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
	std::string demoFileName;

	bool bIsRecording;
	VoxelFrame currentFrame;

	bool bIsPlayback;
	uint32 playbackIndex;
	std::vector<VoxelFrame> playbackFrames;

public:
	virtual ~SpectatorController();

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;

	virtual void Draw(const Window* window, const float& deltaTime);

private:
	void Set(const uint32& x, const uint32& y, const uint32& z, const float& value);
};

