#pragma once
#include "Material.h"
#include "Texture.h"


class InteractionMaterial : public WorldMaterialBase
{
protected:
	///
	/// Cached uniforms
	///
	uint32 m_uniformColour;


public:
	InteractionMaterial();

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
