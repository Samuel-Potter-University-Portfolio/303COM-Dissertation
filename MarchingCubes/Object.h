#pragma once
#include "Common.h"
#include "Transform.h"


class Object
{
protected:
	///
	/// General Vars
	///
	Transform m_transform;


public:
	Object();
	virtual ~Object();

	/**
	* Callback for when any logic should happen
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	virtual void Update(const float& deltaTime);


	///
	/// Getters & Setter
	///
public:
	inline Transform* GetTransform() { return &m_transform; }
	inline const Transform* GetTransform() const { return &m_transform; }
};

