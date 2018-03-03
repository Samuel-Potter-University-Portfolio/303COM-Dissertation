#include "Object.h"



Object::Object()
{
}

Object::~Object()
{
}

void Object::HandleUpdate(const float& deltaTime)
{
	if (bHasBegun)
		Update(deltaTime);
	else
	{
		Begin();
		bHasBegun = true;
	}
}

void Object::Begin()
{
	std::pow(2, 1);
}

void Object::Update(const float& deltaTime)
{
}

void Object::Draw(const class Window* window, const float& deltaTime)
{

}
