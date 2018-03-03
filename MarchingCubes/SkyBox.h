#pragma once
#include "Object.h"
#include "Mesh.h"
#include "SkyboxMaterial.h"


class SkyBox : public Object
{
private:
	///
	/// General Vars
	///
	Mesh* m_mesh = nullptr;
	SkyboxMaterial* m_material = nullptr;

public:
	SkyBox();
	virtual ~SkyBox();

	virtual void Begin() override;
	virtual void Draw(const Window* window, const float& deltaTime) override;
};

