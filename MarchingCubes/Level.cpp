#include "Level.h"



Level::Level()
{
}

Level::~Level()
{
	for (Object* obj : m_objects)
		delete obj;
}

void Level::Load(Engine* engine) 
{
	m_engine = engine;
}

void Level::Update(const float& deltaTime) 
{
	for (Object* obj : m_objects)
		obj->Update(deltaTime);
}