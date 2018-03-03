#pragma once
#include "Common.h"
#include "Shader.h"


class Window;
class Level;
class Mesh;
class Transform;


/**
* Creates an instance of a shader and connect any required processing
* e.g. Storing/Loading uniforms or textures
*/
class Material
{
protected:
	Shader* m_shader = nullptr;

public:
	Material() {};
	virtual ~Material();


	/**
	* Bind this material ready to use
	* @param window				The window that will be rendered to
	* @param level				The level that will be rendered
	*/
	virtual void Bind(const Window* window, const Level* level) = 0;
	/**
	* Unbind all settings changed by this material
	* @param window				The window that will be rendered to
	* @param level				The level that will be rendered
	*/
	virtual void Unbind(const Window* window, const Level* level) = 0;

	/**
	* Prepares this mesh for rendering
	* @param mesh				The mesh that is going to be rendered
	*/
	virtual void PrepareMesh(const Mesh* mesh) = 0;

	/**
	* Render an instance of the previously bound mesh at this transform
	* @param transform			The transform data to use during render
	*/
	virtual void RenderInstance(Transform* transform) = 0;
};



/**
* The base material to be used to world-space based 3D shaders
*/
class WorldMaterialBase : public Material
{
protected:
	///
	/// Cached uniforms
	///
	uint32 m_uniformObjectToWorld;
	uint32 m_uniformWorldToView;
	uint32 m_uniformViewToClip;

	uint32 m_boundMeshTris;

public:
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

	/**
	* Prepares this mesh for rendering
	* @param mesh				The mesh that is going to be rendered
	*/
	virtual void PrepareMesh(const Mesh* mesh) override;

	/**
	* Render an instance of the previously bound mesh at this transform
	* @param transform			The transform data to use during render
	*/
	virtual void RenderInstance(Transform* transform) override;
};