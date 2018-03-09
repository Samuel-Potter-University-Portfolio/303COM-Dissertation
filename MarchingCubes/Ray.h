#pragma once
#include "Common.h"


struct Ray 
{
private:
	vec3 m_origin;
	vec3 m_direction;

public:
	Ray(const vec3& origin = vec3(0,0,0), const vec3& direction = vec3(0,0,1)) 
		: m_origin(origin), m_direction(glm::normalize(direction)) {}

	///
	/// Getters & Setters
	///
public:
	inline void SetOrigin(const vec3& origin) { m_origin = origin; }
	inline const vec3& GetOrigin() const { return m_origin; }

	inline void SetDirection(const vec3& direction) { m_direction = glm::normalize(direction); }
	inline const vec3& GetDirection() const { return m_direction; }
};
