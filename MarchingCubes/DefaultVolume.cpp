#include "DefaultVolume.h"
#include "MarchingCubes.h"

#include <unordered_map>
#include "DefaultMaterial.h"
#include "MeshBuilder.h"
#include "Window.h"
#include "InteractionMaterial.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\vector_angle.hpp>


DefaultVolume::DefaultVolume()
{
	m_isoLevel = 0.15f;
}

DefaultVolume::~DefaultVolume()
{
	if (m_data != nullptr)
		delete[] m_data;
	if (m_mesh != nullptr)
		delete m_mesh;
	if (m_material != nullptr)
		delete m_material;
}


///
/// Volume Data Overrides
///
void DefaultVolume::Init(const uvec3& resolution, const vec3& scale)
{
	m_data = new float[resolution.x * resolution.y * resolution.z]{ DEFAULT_VALUE };
	m_resolution = resolution;
	m_scale = scale;
}

void DefaultVolume::Set(uint32 x, uint32 y, uint32 z, float value) 
{
	m_data[GetIndex(x, y, z)] = value;
	bRequiresRebuild = true;
}

float DefaultVolume::Get(uint32 x, uint32 y, uint32 z) 
{
	return m_data[GetIndex(x, y, z)];
}


///
/// Object functions
///

void DefaultVolume::Begin() 
{
	m_material = new DefaultMaterial;
	m_wireMaterial = new InteractionMaterial(vec4(0, 1, 0, 1));

	m_mesh = new Mesh;
	m_mesh->MarkDynamic();

	BuildMesh();
}

void DefaultVolume::Update(const float& deltaTime) 
{
	// Rebuild mesh if it needs it
	if (bRequiresRebuild)
	{
		BuildMesh();
		bRequiresRebuild = false;
	}
}

void DefaultVolume::Draw(const class Window* window, const float& deltaTime) 
{
	static bool drawWireFrame = false;
	static bool drawMainObject = true;
	const Keyboard* keyboard = window->GetKeyboard();

	if (keyboard->IsKeyPressed(Keyboard::Key::KV_T))
		drawWireFrame = !drawWireFrame;
	if (keyboard->IsKeyPressed(Keyboard::Key::KV_R))
		drawMainObject = !drawMainObject;


	if (m_mesh != nullptr)
	{
		Transform t;

		if (drawMainObject)
		{
			m_material->Bind(window, GetLevel());
			m_material->PrepareMesh(m_mesh);
			m_material->RenderInstance(&t);
			m_material->Unbind(window, GetLevel());
		}

		if (drawWireFrame)
		{
			m_wireMaterial->Bind(window, GetLevel());
			m_wireMaterial->PrepareMesh(m_mesh);
			m_wireMaterial->RenderInstance(&t);
			m_wireMaterial->Unbind(window, GetLevel());
		}
	}
}

#include "Logger.h"
void DefaultVolume::BuildMesh() 
{
	MeshBuilderMinimal builder;
	vec3 edges[12];
	
	for (uint32 x = 0; x < GetResolution().x - 1; ++x)
		for (uint32 y = 0; y < GetResolution().y - 1; ++y)
			for (uint32 z = 0; z < GetResolution().z - 1; ++z)
			{
				// Encode case based on bit presence
				uint8 caseIndex = 0;
				if (Get(x + 0, y + 0, z + 0) >= m_isoLevel) caseIndex |= 1;
				if (Get(x + 1, y + 0, z + 0) >= m_isoLevel) caseIndex |= 2;
				if (Get(x + 1, y + 0, z + 1) >= m_isoLevel) caseIndex |= 4;
				if (Get(x + 0, y + 0, z + 1) >= m_isoLevel) caseIndex |= 8;
				if (Get(x + 0, y + 1, z + 0) >= m_isoLevel) caseIndex |= 16;
				if (Get(x + 1, y + 1, z + 0) >= m_isoLevel) caseIndex |= 32;
				if (Get(x + 1, y + 1, z + 1) >= m_isoLevel) caseIndex |= 64;
				if (Get(x + 0, y + 1, z + 1) >= m_isoLevel) caseIndex |= 128;
			

				// Fully inside iso-surface
				if (caseIndex == 0 || caseIndex == 255)
					continue;

				// Smooth edges based on density
#define VERT_LERP(x0, y0, z0, x1, y1, z1) MC::VertexLerp(m_isoLevel, vec3(x + x0,y + y0,z + z0), vec3(x + x1, y + y1, z + z1), Get(x + x0, y + y0, z + z0), Get(x + x1, y + y1, z + z1))
				
				if (MC::CaseRequiredEdges[caseIndex] & 1)
					edges[0] = VERT_LERP(0,0,0, 1,0,0);
				if (MC::CaseRequiredEdges[caseIndex] & 2)
					edges[1] = VERT_LERP(1,0,0, 1,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 4)
					edges[2] = VERT_LERP(0,0,1, 1,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 8)
					edges[3] = VERT_LERP(0,0,0, 0,0,1);
				if (MC::CaseRequiredEdges[caseIndex] & 16)
					edges[4] = VERT_LERP(0,1,0, 1,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 32)
					edges[5] = VERT_LERP(1,1,0, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 64)
					edges[6] = VERT_LERP(0,1,1, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 128)
					edges[7] = VERT_LERP(0,1,0, 0,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 256)
					edges[8] = VERT_LERP(0,0,0, 0,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 512)
					edges[9] = VERT_LERP(1,0,0, 1,1,0);
				if (MC::CaseRequiredEdges[caseIndex] & 1024)
					edges[10] = VERT_LERP(1,0,1, 1,1,1);
				if (MC::CaseRequiredEdges[caseIndex] & 2048)
					edges[11] = VERT_LERP(0,0,1, 0,1,1);


				// Add triangles for this case
				int8* caseEdges = MC::Cases[caseIndex];
				while (*caseEdges != -1)
				{
					int8 edge0 = *(caseEdges++);
					int8 edge1 = *(caseEdges++);
					int8 edge2 = *(caseEdges++);

					const vec3& A = edges[edge0];
					const vec3& B = edges[edge1];
					const vec3& C = edges[edge2];

					// Ignore triangle which is malformed
					if (A == B || A == C || B == C)
						continue;


					const vec3 normal = glm::cross(B - A, C - A);
					const float normalLengthSqrd = dot(normal, normal);

					// If normal is 0 it means the edge has been moved so the face is now a line
					if (normalLengthSqrd != 0.0f && !std::isnan(normalLengthSqrd))
					{
						const uint32 a = builder.AddVertex(A, normal);
						const uint32 b = builder.AddVertex(B, normal);
						const uint32 c = builder.AddVertex(C, normal);
						builder.AddTriangle(a, b, c);
					}
				}
			}


	//builder.PerformEdgeCollapseReduction(10000);
	builder.BuildMesh(m_mesh);
}