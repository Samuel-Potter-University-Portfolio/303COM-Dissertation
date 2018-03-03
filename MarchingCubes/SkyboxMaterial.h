#pragma once
#include "Material.h"
#include "Texture.h"


class SkyboxMaterial : public WorldMaterialBase
{
protected:
	///
	/// Cached uniforms
	///

	///
	/// Material Vars
	///
	Texture* m_skyTexture;

public:
	SkyboxMaterial();
	virtual ~SkyboxMaterial();

	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param level				The level that will be rendered
	*/
	virtual void Bind(const Window* window, const Level* level) override;
	/**
	* Unbind all settings changed by this material
	* @param window				The window that will be rendered to
	* @param level				The level that will be rendered
	*/
	virtual void Unbind(const Window* window, const Level* level) override;
};

