#pragma once
#include "Common.h"
#include "Object.h"
#include "Camera.h"
#include <vector>


/**
* Container for objects
*/
class Level
{
private:
	///
	/// General settings
	///
	class Engine* m_engine;
	Camera* m_camera;

	std::vector<Object*> m_objects;

public:
	Level();
	~Level();

	/**
	* Callback for when this level is loaded into the engine
	* @param engine			The engine which will drive this level
	*/
	void Load(Engine* engine);

	/**
	* Called by the engine to distribute 
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void HandleUpdate(const float& deltaTime);

	/**
	* Add an object to the level
	* @param obj			The object to add (Level will manage memory)
	*/
	Object* AddObject(Object* obj);


	///
	/// Getters & Setters
	///
public:
	inline Engine* GetEngine() const { return m_engine; }
	inline Camera* GetCamera() const { return m_camera; }


	/**
	* Retreive first object which have the given tag
	* @param tag			The tag to search for
	* @return Vector containing all objects
	*/
	Object* FindObjectWithTag(const uint32& tag) const;
	/**
	* Retreive all objects which have the given tag
	* @param tag			The tag to search for
	* @return Vector containing all objects
	*/
	std::vector<Object*> FindObjectsWithTag(const uint32& tag) const;

	/**
	* Retreive first object which have the given name
	* @param name			The name to search for
	* @return Vector containing all objects
	*/
	Object* FindObjectWithName(const string& name) const;
	/**
	* Retreive all objects which have the given name
	* @param name			The name to search for
	* @return Vector containing all objects
	*/
	std::vector<Object*> FindObjectsWithName(const string& name) const;

	/**
	* Retreive first object which have the given name
	* @param T				Type to search for
	* @return Vector containing all objects
	*/
	template<typename T>
	T* FindObject() const 
	{
		for (Object* obj : m_objects)
		{
			T* c = dynamic_cast<T*>(obj);
			if (c != nullptr)
				return c;
		}

		return nullptr;
	}
	/**
	* Retreive all objects which have the given name
	* @param T				Type to search for
	* @return Vector containing all objects
	*/
	template<typename T>
	std::vector<T*> FindObjects() const 
	{
		std::vector<T*> output;

		for (Object* obj : m_objects) 
		{
			T* c = dynamic_cast<T*>(obj);
			if (c != nullptr)
				output.push_back(c);
		}

		return output;
	}

};

