#include "Engine.h"

#include "Level.h"
#include "Logger.h"

#include "SkyBox.h"
#include "SpectatorController.h"

#include "DefaultVolume.h"
#include "ChunkedVolume.h"
#include "OctreeVolume.h"
#include "OctreeRepVolume.h"
#include "LayeredVolume.h"



/*
#include "DefaultMaterial.h"
#include "MarchingCubes.h"
#include <iostream>
#include "InteractionMaterial.h"
class TestObj : public Object 
{
public:
	Mesh* mesh;
	Material* material;

	Mesh* debugmesh;
	Material* debugmaterial;

	std::vector<uint8> link;
	uint8 m_caseIndex = 233;

	TestObj() 
	{
	}

	virtual void Begin() override 
	{
		mesh = new Mesh;
		mesh->MarkDynamic();

		debugmesh = new Mesh;
		debugmesh->MarkDynamic();

		debugmesh->SetVertices(std::vector<vec3>(
		{
			//vec3(-10.0f, -10.0f, -10.0f),
			//vec3(10.0f, -10.0f, -10.0f),
			//vec3(-10.0f, -10.0f, 10.0f),
			//vec3(10.0f, -10.0f, 10.0f),
			//
			//vec3(-10.0f, 10.0f, -10.0f),
			//vec3(10.0f, 10.0f, -10.0f),
			//vec3(-10.0f, 10.0f, 10.0f),
			//vec3(10.0f, 10.0f, 10.0f),

			vec3(0.0f, 0.0f, 0.0f),
			vec3(10.0f, 0.0f, 0.0f),
			vec3(0.0f, 0.0f, 10.0f),
			vec3(10.0f, 0.0f, 10.0f),

			vec3(0.0f, 10.0f, 0.0f),
			vec3(10.0f, 10.0f, 0.0f),
			vec3(0.0f, 10.0f, 10.0f),
			vec3(10.0f, 10.0f, 10.0f),
		}));
		debugmesh->SetTriangles(std::vector<uint32>(
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

		material = new InteractionMaterial;
		debugmaterial = new InteractionMaterial(vec4(0, 0, 0, 1));

		MeshBuilderMinimal builder;
		Build(builder);
		builder.BuildMesh(mesh);
	}

	virtual void Update(const float& deltaTime) override
	{
		if (m_caseIndex == 0)
		{
			for (uint8 i : link)
				std::cout << (int)i << ", ";
			int n = 0;
			n++;
		}

		const auto& keyboard = GetLevel()->GetEngine()->GetWindow()->GetKeyboard();
		if (keyboard->IsKeyPressed(Keyboard::Key::KV_ENTER))// ADD
		{
			link.push_back(m_caseIndex);
			m_caseIndex++;
			MeshBuilderMinimal builder;
			Build(builder);
			builder.BuildMesh(mesh);
		}
		if (keyboard->IsKeyPressed(Keyboard::Key::KV_BACKSPACE))// DONT
		{
			m_caseIndex++;
			MeshBuilderMinimal builder;
			Build(builder);
			builder.BuildMesh(mesh);
		}

		LOG("%i", m_caseIndex);
	}

	virtual void Draw(const Window * window, const float & deltaTime) override
	{
		static bool drawWireFrame = false;
		static bool drawMainObject = true;
		const Keyboard* keyboard = window->GetKeyboard();

		if (keyboard->IsKeyPressed(Keyboard::Key::KV_T))
			drawWireFrame = !drawWireFrame;
		if (keyboard->IsKeyPressed(Keyboard::Key::KV_R))
			drawMainObject = !drawMainObject;


		if (debugmesh != nullptr)
		{
			Transform t;

			debugmaterial->Bind(window, GetLevel());
			debugmaterial->PrepareMesh(debugmesh);
			debugmaterial->RenderInstance(&t);
			debugmaterial->Unbind(window, GetLevel());
		}

		if (mesh != nullptr)
		{
			Transform t;

			material->Bind(window, GetLevel());
			material->PrepareMesh(mesh);
			material->RenderInstance(&t);
			material->Unbind(window, GetLevel());
		}
	}


	void Build(MeshBuilderMinimal& builder)
	{
	
		// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) (vec3(x0 + x1, y0 + y1, z0 + z1) * 0.5f) * 10.0f;
		vec3 temp;
		vec3 edges[12];


		if (MC::CaseRequiredEdges[m_caseIndex] & 1)
			edges[0] = VERT_LERP(0, 0, 0, 1, 0, 0);
		if (MC::CaseRequiredEdges[m_caseIndex] & 2)
			edges[1] = VERT_LERP(1, 0, 0, 1, 0, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 4)
			edges[2] = VERT_LERP(0, 0, 1, 1, 0, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 8)
			edges[3] = VERT_LERP(0, 0, 0, 0, 0, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 16)
			edges[4] = VERT_LERP(0, 1, 0, 1, 1, 0);
		if (MC::CaseRequiredEdges[m_caseIndex] & 32)
			edges[5] = VERT_LERP(1, 1, 0, 1, 1, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 64)
			edges[6] = VERT_LERP(0, 1, 1, 1, 1, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 128)
			edges[7] = VERT_LERP(0, 1, 0, 0, 1, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 256)
			edges[8] = VERT_LERP(0, 0, 0, 0, 1, 0);
		if (MC::CaseRequiredEdges[m_caseIndex] & 512)
			edges[9] = VERT_LERP(1, 0, 0, 1, 1, 0);
		if (MC::CaseRequiredEdges[m_caseIndex] & 1024)
			edges[10] = VERT_LERP(1, 0, 1, 1, 1, 1);
		if (MC::CaseRequiredEdges[m_caseIndex] & 2048)
			edges[11] = VERT_LERP(0, 0, 1, 0, 1, 1);



		// Add triangles for this case
		int8* caseEdges = MC::Cases[m_caseIndex];
		while (*caseEdges != -1)
		{
			int8 edge0 = *(caseEdges++);
			int8 edge1 = *(caseEdges++);
			int8 edge2 = *(caseEdges++);

			vec3 normal = glm::cross(edges[edge1] - edges[edge0], edges[edge2] - edges[edge0]);

			const uint32 a = builder.AddVertex(edges[edge0], normal);
			const uint32 b = builder.AddVertex(edges[edge1], normal);
			const uint32 c = builder.AddVertex(edges[edge2], normal);
			builder.AddTriangle(a, b, c);
		}
	}
};
//*/

#include "MarchingCubes.h"
#include <iostream>

void DebugFuncCreateSimpleCases() 
{
	for (uint32 i = 0; i < 255; ++i)
	{
		// Check all faces
		uint32 r0 = (MC::CaseRequiredEdges[i] & 1) ? 1 : 0;
		uint32 r1 = (MC::CaseRequiredEdges[i] & 2) ? 1 : 0;
		uint32 r2 = (MC::CaseRequiredEdges[i] & 4) ? 1 : 0;
		uint32 r3 = (MC::CaseRequiredEdges[i] & 8) ? 1 : 0;
		uint32 r4 = (MC::CaseRequiredEdges[i] & 16) ? 1 : 0;
		uint32 r5 = (MC::CaseRequiredEdges[i] & 32) ? 1 : 0;
		uint32 r6 = (MC::CaseRequiredEdges[i] & 64) ? 1 : 0;
		uint32 r7 = (MC::CaseRequiredEdges[i] & 128) ? 1 : 0;
		uint32 r8 = (MC::CaseRequiredEdges[i] & 256) ? 1 : 0;
		uint32 r9 = (MC::CaseRequiredEdges[i] & 512) ? 1 : 0;
		uint32 r10 = (MC::CaseRequiredEdges[i] & 1024) ? 1 : 0;
		uint32 r11 = (MC::CaseRequiredEdges[i] & 2048) ? 1 : 0;

		if ((r0 + r1 + r2 + r3) > 2)
			continue;
		if ((r3 + r11 + r7 + r8) > 2)
			continue;
		if ((r0 + r9 + r4 + r8) > 2)
			continue;
		if ((r1 + r9 + r5 + r10) > 2)
			continue;
		if ((r2 + r10 + r6 + r11) > 2)
			continue;
		if ((r4 + r5 + r6 + r7) > 2)
			continue;
		
		std::cout << (uint32)i << ", ";
	}

	return;
}



int main(int argc, char** argv)
{
	EngineInit settings;
	settings.Title = "Marching Cubes";
	Engine engine(settings);
	Level* level = new Level;

	level->AddObject(new SpectatorController);
	level->AddObject(new SkyBox);
	//level->AddObject(new LayeredVolume);
	level->AddObject(new DefaultVolume);
	//level->AddObject(new TestObj);
	engine.SetLevel(level);
	engine.LaunchMainLoop();
	return 0;
}
