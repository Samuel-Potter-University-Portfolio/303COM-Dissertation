#include "Level.h"
#include "Logger.h"
#include "Engine.h"


Level::Level()
{
	m_camera = new Camera;
	m_camera->SetNearPlane(0.1f);
	m_camera->SetFarPlane(1000.0f);
}

Level::~Level()
{
	for (Object* obj : m_objects)
		delete obj;
	delete m_camera;
	LOG("Level Destroyed.");
}

void Level::Load(Engine* engine) 
{
	m_engine = engine;
}

void Level::HandleUpdate(const float& deltaTime)
{
	// Update
	for (Object* obj : m_objects)
		obj->HandleUpdate(deltaTime);

	// Render
	glClearColor(0.1451f, 0.1490f, 0.1922f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Window* window = m_engine->GetWindow();

	for (Object* obj : m_objects)
		obj->Draw(window, deltaTime);
}

Object* Level::AddObject(Object* obj) 
{
	m_objects.push_back(obj);
	obj->m_level = this;
	return obj;
}


Object* Level::FindObjectWithTag(const uint32& tag) const
{
	for (Object* obj : m_objects)
		if (obj->HasTag(tag))
			return obj;

	return nullptr;
}

std::vector<Object*> Level::FindObjectsWithTag(const uint32& tag) const
{
	std::vector<Object*> output;

	for (Object* obj : m_objects)
		if(obj->HasTag(tag))
			output.push_back(obj);

	return output;
}


Object* Level::FindObjectWithName(const string& name) const 
{
	for (Object* obj : m_objects)
		if (obj->GetName() == name)
			return obj;

	return nullptr;
}

std::vector<Object*> Level::FindObjectsWithName(const string& name) const 
{
	std::vector<Object*> output;

	for (Object* obj : m_objects)
		if(obj->GetName() == name)
			output.push_back(obj);

	return output;
}