#include "SpectatorController.h"
#include "Level.h"
#include "Engine.h"

#include "Logger.h"


void SpectatorController::Begin() 
{
	camera = GetLevel()->GetCamera();
	LOG("Spawned in SpectatorController");
}

void SpectatorController::Update(const float& deltaTime) 
{
	Keyboard* keyboard = GetEngine()->GetWindow()->GetKeyboard();
	Mouse* mouse = GetEngine()->GetWindow()->GetMouse();

	const float speed = 30.0f * deltaTime;


	if (keyboard->IsKeyDown(Keyboard::Key::KV_W))
		m_transform.Translate(camera->GetForward() * speed);
	if (keyboard->IsKeyDown(Keyboard::Key::KV_S))
		m_transform.Translate(-camera->GetForward() * speed);

	if (keyboard->IsKeyDown(Keyboard::Key::KV_D))
		m_transform.Translate(camera->GetRight() * speed);
	if (keyboard->IsKeyDown(Keyboard::Key::KV_A))
		m_transform.Translate(-camera->GetRight() * speed);

	if (keyboard->IsKeyDown(Keyboard::Key::KV_SPACE))
		m_transform.Translate(vec3(0, speed, 0));
	if (keyboard->IsKeyDown(Keyboard::Key::KV_LCONTROL))
		m_transform.Translate(vec3(0, -speed, 0));


	if (mouse->IsButtonPressed(Mouse::Button::MB_LEFT))
		mouse->SetGrabbed(!mouse->IsGrabbed());


	if (mouse->IsGrabbed())
	{
		const float turn = 1.18f * 0.05f;
		const vec2 vel = mouse->GetVelocity();

		vec3 rotation = m_transform.GetEularRotation() + vec3(-vel.y * turn, -vel.x * turn, 0.0f);
		if (rotation.x < -89.0f)
			rotation.x = -89.0f;
		if (rotation.x > 89.0f)
			rotation.x = 89.0f;

		rotation.y = std::fmodf(rotation.y, 360.0f);
		if (rotation.y < 0.0f)
			rotation.y = 360.0f - rotation.y;

		m_transform.SetEularRotation(rotation);
	}

	camera->SetLocation(m_transform.GetLocation());
	camera->SetEularRotation(m_transform.GetEularRotation());
}