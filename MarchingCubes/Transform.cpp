#include "Transform.h"

#include "gtc\matrix_transform.hpp"


Transform::Transform() :
	m_location(0, 0, 0),
	m_rotation(0, 0, 0),
	m_scale(1, 1, 1)
{
}

mat4 Transform::GetTransformMatrix()
{
	// Only rebuild if something has changed
	if (!bRequiresRebuild)
		return m_matrix;

	m_matrix = mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_location);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), vec3(1, 0, 0));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), vec3(0, 1, 0));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), vec3(0, 0, 1));
	m_matrix = glm::scale(m_matrix, m_scale);
	bRequiresRebuild = false;
	return m_matrix;
}

vec3 Transform::GetForward()
{
	return glm::normalize(vec3(
		sin(glm::radians(m_rotation.y)),
		tan(glm::radians(m_rotation.x)),
		cos(glm::radians(m_rotation.y))
	));
}
vec3 Transform::GetForward2D()
{
	return glm::normalize(vec3(
		sin(glm::radians(m_rotation.y)),
		0,
		cos(glm::radians(m_rotation.y))
	));
}

vec3 Transform::GetRight()
{
	return glm::cross(GetForward(), vec3(0, 1, 0));
}
vec3 Transform::GetRight2D()
{
	return glm::cross(GetForward2D(), vec3(0, 1, 0));
}

vec3 Transform::GetUp()
{
	return glm::cross(GetForward(), -GetRight());
}