#pragma once
#include "Common.h"
#include "Object.h"
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
	* Callback for when any logic should happen
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void Update(const float& deltaTime);
};

