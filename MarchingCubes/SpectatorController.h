#pragma once
#include "Object.h"
#include "Camera.h"


class SpectatorController : public Object
{
private:
	///
	/// General Vars
	///
	Camera* camera;


public:

	virtual void Begin() override;
	virtual void Update(const float& deltaTime) override;
};

