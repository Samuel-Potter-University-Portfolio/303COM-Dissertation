#include "Level.h"
#include "Logger.h"


Level::Level()
{
}

Level::~Level()
{
	for (Object* obj : m_objects)
		delete obj;
	LOG("Level Destroyed.");
}

void Level::Load(Engine* engine) 
{
	m_engine = engine;
}

void Level::HandleUpdate(const float& deltaTime)
{
	for (Object* obj : m_objects)
		obj->HandleUpdate(deltaTime);
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