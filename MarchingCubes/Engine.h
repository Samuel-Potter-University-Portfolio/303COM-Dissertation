#pragma once
#include "Common.h"
#include "Window.h"
#include "Level.h"


/**
* Initialization settings for a engine
*/
struct EngineInit
{
	string	Title = "Window";
};


class Engine
{
private:
	///
	/// General Settings
	/// 
	Window* m_window = nullptr;
	EngineInit m_settings;
	Level* m_currentLevel = nullptr;

public:
	Engine(const EngineInit& settings);
	~Engine();
	
	/**
	* Launches this engine into it's main loop
	*/
	void LaunchMainLoop();

private:
	/**
	* Callback for when any logic should happen
	* @param window				The window to render to
	* @param deltaTime			Time (In seconds) since last update/render
	*/
	void Update(Window& window, const float& deltaTime);


	///
	/// Getters & Setters
	///
public:
	inline Window* GetWindow() const { return m_window; }

	inline void SetLevel(Level* level) { m_currentLevel = level; level->Load(this); }
	inline Level* GetLevel() const { return m_currentLevel; }
};

