#include "SpectatorController.h"
#include "Level.h"
#include "Engine.h"

#include "Mesh.h"
#include "InteractionMaterial.h"
#include "Logger.h"


void SpectatorController::Begin() 
{
	camera = GetLevel()->GetCamera();

	m_mesh = new Mesh;
	m_mesh->SetVertices(std::vector<vec3>(
	{
		/*vec3(-1.0f, -1.0f, -1.0f),
		vec3(1.0f, -1.0f, -1.0f),
		vec3(-1.0f, -1.0f, 1.0f),
		vec3(1.0f, -1.0f, 1.0f),

		vec3(-1.0f, 1.0f, -1.0f),
		vec3(1.0f, 1.0f, -1.0f),
		vec3(-1.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),*/

		vec3(0.0f, 0.0f, 0.0f),
		vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(1.0f, 0.0f, 1.0f),

		vec3(0.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f),
		vec3(0.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),
	}));
	m_mesh->SetTriangles(std::vector<uint32>(
	{
		0,1,2, 2,1,3,
		4,6,5, 5,6,7,
		0,2,1, 2,3,1,
		4,5,6, 5,7,6,

		2,3,6, 6,3,7,
		3,1,7, 1,5,7,
		2,6,3, 6,7,3,
		3,7,1, 1,7,5,

		1,0,4, 1,4,5,
		0,2,4, 2,6,4,
		1,4,0, 1,5,4,
		0,4,2, 2,4,6,

	}));

	m_material = new InteractionMaterial;

	// Set default volume
	LOG("Setting up initial volume");
	IVoxelVolume* volume = GetLevel()->FindObject<IVoxelVolume>();
	if (volume != nullptr)
	{
		//volume->LoadFromPvmFile("Resources/Lobster.pvm");
	
		// Sphere
		//*
		const uint32 radius = 32;
		const uint32 diametre = radius * 2;
		volume->Init(uvec3(diametre, diametre, diametre), vec3(1, 1, 1));

		for (int x = 0; x < diametre; ++x)
			for (int y = 0; y < diametre; ++y)
				for (int z = 0; z < diametre; ++z)
				{
					float distance = glm::length(vec3(x, y, z) - vec3(radius, radius, radius));
					float v = 1.0f - glm::clamp(distance / (float)radius, 0.0f, 1.0f);
					volume->Set(x, y, z, v);
				}
				//*/

		// Flat platform
		/*
		volume->Init(uvec3(64, 64, 64), vec3(1, 1, 1));
		for (int x = 0; x < 64; ++x)
			for (int z = 0; z < 64; ++z)
			{
				volume->Set(x, 63, z, 1.0f);
				volume->Set(x, 7, z, 0.5f);
				volume->Set(x, 0, z, 1.0f);
			}
			//*/
	}
}

SpectatorController::~SpectatorController() 
{
	if (m_mesh != nullptr)
		delete m_mesh;
	if (m_material != nullptr)
		delete m_material;
}

void SpectatorController::Update(const float& deltaTime) 
{
	const Keyboard* keyboard = GetEngine()->GetWindow()->GetKeyboard();
	Mouse* mouse = GetEngine()->GetWindow()->GetMouse();
	IVoxelVolume* volume = GetLevel()->FindObject<IVoxelVolume>();

	const float speed = 30.0f * deltaTime;

	// Movement
	{
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



		if (mouse->IsButtonPressed(Mouse::Button::MB_MIDDLE))
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

	// Interaction
	{
		// Fetch for new interaction whenever mouse moves
		if (true || dot(mouse->GetVelocity(), mouse->GetVelocity()) > 0) // TODO - Take keyboard into account
		{
			bLookingAtVoxel = false;

			if (volume != nullptr)
				bLookingAtVoxel = volume->Raycast(Ray(m_transform.GetLocation(), m_transform.GetForward()), lookatVoxel, 1000.0f);
		}

		const float interactionRate = 5.0f * deltaTime;

		// Voxel interaction
		if (bLookingAtVoxel)
		{
			// Destroy voxel
			//if (mouse->IsButtonDown(Mouse::Button::MB_LEFT))
			//	volume->Set(lookatVoxel.coord.x, lookatVoxel.coord.y, lookatVoxel.coord.z, glm::clamp(lookatVoxel.value - interactionRate, 0.0f, 1.0f));

			// Place voxel
			//if (mouse->IsButtonDown(Mouse::Button::MB_RIGHT))
			//{
			//	if (lookatVoxel.value < 1.0f)
			//		volume->Set(lookatVoxel.coord.x, lookatVoxel.coord.y, lookatVoxel.coord.z, glm::clamp(lookatVoxel.value + interactionRate, 0.0f, 1.0f));
			//	else
			//		volume->Set(lookatVoxel.surface.x, lookatVoxel.surface.y, lookatVoxel.surface.z, glm::clamp(lookatVoxel.surfaceValue + interactionRate, 0.0f, 1.0f));
			//}

			// Destroy/Destroy voxel
			const bool destroy = mouse->IsButtonPressed(Mouse::Button::MB_LEFT);
			const bool place = mouse->IsButtonPressed(Mouse::Button::MB_RIGHT);
			if (destroy || place)
			{
				const uvec3 res = volume->GetResolution();
				const int32 radius = 5;

				for (int32 x = -radius; x <= radius; ++x)
					for (int32 y = -radius; y <= radius; ++y)
						for (int32 z = -radius; z <= radius; ++z)
						{
							const float length = glm::length(vec3(x, y, z)) / radius;
							
							ivec3 pos(
								lookatVoxel.coord.x + x,
								lookatVoxel.coord.y + y,
								lookatVoxel.coord.z + z
							);

							if (pos.x >= 0 && pos.x < res.x && pos.y >= 0 && pos.y < res.y && pos.z >= 0 && pos.z < res.z)
							{
								if (destroy)
								{
									float value = volume->Get(pos.x, pos.y, pos.z);
									volume->Set(pos.x, pos.y, pos.z, glm::min(1.0f, glm::min(length * volume->GetIsoLevel(), value)));
								}
								if (place)
								{
									float value = volume->Get(pos.x, pos.y, pos.z);
									volume->Set(pos.x, pos.y, pos.z, glm::max(0.0f, glm::max(1.0f - length, value)));
								}

							}
						}
			}
		}
	}
}

void SpectatorController::Draw(const Window* window, const float& deltaTime) 
{
	if (bLookingAtVoxel || true)
	{
		m_material->Bind(window, GetLevel());
		m_material->PrepareMesh(m_mesh);

		Transform t;
		t.SetLocation(lookatVoxel.coord);
		m_material->RenderInstance(&t);
		t.SetLocation(lookatVoxel.surface);
		m_material->RenderInstance(&t);
		m_material->Unbind(window, GetLevel());
	}
}