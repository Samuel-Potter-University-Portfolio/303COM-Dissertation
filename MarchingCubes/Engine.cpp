#include "Engine.h"
#include "Logger.h"


Engine::Engine(const EngineInit& settings) : m_settings(settings)
{
	m_window = new Window;
}

Engine::~Engine()
{
	delete m_window;
	delete m_currentLevel;
	LOG("Engine Destroyed.");
}

void Engine::LaunchMainLoop() 
{
	LOG("Attempting to launch '%s'", m_settings.Title.c_str());

	if (!Window::InitAPI())
		return;

	WindowInit settings;
	settings.Major = 4;
	settings.Minor = 2;
	settings.Title = m_settings.Title;
	settings.Width = 1280;
	settings.Height = 720;
	settings.bVerticalSync = true;

	if (!m_window->Open(settings))
	{
		LOG_ERROR("Failed to open window");
		return;
	}
	
	m_window->LaunchMainLoop(std::bind(&Engine::Update, this, std::placeholders::_1, std::placeholders::_2));
	Window::DestroyAPI();
}

void Engine::Update(Window& window, const float& deltaTime) 
{
	if (m_currentLevel != nullptr)
		m_currentLevel->HandleUpdate(deltaTime);
}