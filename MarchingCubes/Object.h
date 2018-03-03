#pragma once
#include "Common.h"
#include "Transform.h"


#define TAG_Env 2 << 0;



class Object
{
private:
	friend class Level;

	///
	/// Private Vars
	///
	uint32 m_tags = 0;
	bool bHasBegun = false;

	string m_name;
	Level* m_level = nullptr;

protected:
	///
	/// General Vars
	///
	Transform m_transform;

public:
	Object();
	virtual ~Object();

	/**
	* Called by the engine to distribute 
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void HandleUpdate(const float& deltaTime);


	/**
	* Called when this object initially gets loaded
	*/
	virtual void Begin();
	/**
	* Called every frame
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	virtual void Update(const float& deltaTime);

	/**
	* Called when this object should be rendered
	* @param window				The window which is being drawn to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	virtual void Draw(const class Window* window, const float& deltaTime);



	///
	/// Getters & Setter
	///
public:
	inline Transform* GetTransform() { return &m_transform; }
	inline const Transform* GetTransform() const { return &m_transform; }

	inline void SetName(const string& name) { m_name = name; }
	inline string GetName() const { return m_name; }

	inline void AddTag(const uint32& tag) { m_tags |= tag; }
	inline bool HasTag(const uint32& tag) const { return (m_tags & tag) != 0; }

	inline Level* GetLevel() const { return m_level; }
};

