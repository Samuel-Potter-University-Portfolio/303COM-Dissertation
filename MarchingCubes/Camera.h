#pragma once
#include "Transform.h"


class Window;


/**
* Holds all of the information about the camera view
*/
class Camera : public Transform
{
private:
	float m_fov = 70.0f;
	float m_nearPlane = 0.1f;
	float m_farPlane = 1000.0f;

	///
	/// Only rebuild matrix, if there has been a change
	///
	mat4 m_perspectiveMatrix;
	bool bRequiresPerspectiveRebuild = true;
	float m_perspectiveLastAspect;

public:
	/**
	* Get the view matrix to use for this camera data
	*/
	mat4 GetViewMatrix();
	/**
	* Get the perspective matrix to use for this camera data
	* @param window			The window settings to use to build the window
	*/
	mat4 GetPerspectiveMatrix(const Window* window);


	///
	/// Getters & Setters
	///
public:
	inline void SetFoV(const float& value) { m_fov = value; bRequiresPerspectiveRebuild = true; }
	inline float GetFoV() const { return m_fov; }

	inline void SetNearPlane(const float& value) { m_nearPlane = value; bRequiresPerspectiveRebuild = true; }
	inline float GetNearPlane() const { return m_nearPlane; }

	inline void SetFarPlane(const float& value) { m_farPlane = value; bRequiresPerspectiveRebuild = true; }
	inline float GetFarPlane() const { return m_farPlane; }
};

