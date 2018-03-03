#include "Camera.h"
#include "Window.h"

#include "gtc\matrix_transform.hpp"


mat4 Camera::GetViewMatrix()
{
	const vec3 loc = GetLocation();
	return glm::lookAt(loc, loc + GetForward(), vec3(0, 1, 0));
}

mat4 Camera::GetPerspectiveMatrix(const Window* window)
{
	// Force rebuild if window has changed size
	const float aspect = window->GetAspectRatio();
	if (m_perspectiveLastAspect != aspect)
	{
		m_perspectiveLastAspect = aspect;
		bRequiresPerspectiveRebuild = true;
	}


	// Only rebuild if something has changed
	if (!bRequiresPerspectiveRebuild)
		return m_perspectiveMatrix;

	m_perspectiveMatrix = glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
	bRequiresPerspectiveRebuild = false;
	return m_perspectiveMatrix;
}