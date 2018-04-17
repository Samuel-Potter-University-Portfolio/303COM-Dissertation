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
	vec4 m_colour;


public:
	InteractionMaterial(const vec4& colour = vec4(1.0f, 1.0f, 1.0f, 1.0f));

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


	///
	/// Getters & Setters
	///
public:
	inline void SetColour(const vec4& colour) { m_colour = colour; }
	inline void SetColour(const vec3& colour) { m_colour = vec4(colour, 1.0f); }

	inline const vec4& GetColour() const { return m_colour; }
};
