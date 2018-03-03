#include "Material.h"

#include "GL/glew.h"
#include "Level.h"
#include "Mesh.h"
#include "Transform.h"


Material::~Material() 
{
	if (m_shader != nullptr)
		delete m_shader;
}


void WorldMaterialBase::Bind(const Window* window, const Level* level)
{
	if (m_shader == nullptr)
		return;

	m_shader->Bind();

	if (m_uniformObjectToWorld == 0)
		m_uniformObjectToWorld = m_shader->GetUniform("ObjectToWorld");

	if (m_uniformWorldToView == 0)
		m_uniformWorldToView = m_shader->GetUniform("WorldToView");

	if (m_uniformViewToClip == 0)
		m_uniformViewToClip = m_shader->GetUniform("ViewToClip");


	m_shader->SetUniformMat4(m_uniformWorldToView, level->GetCamera()->GetViewMatrix());
	m_shader->SetUniformMat4(m_uniformViewToClip, level->GetCamera()->GetPerspectiveMatrix(window));
}

void WorldMaterialBase::Unbind(const Window* window, const Level* level)
{
	if (m_shader == nullptr)
		return;

	m_shader->Unbind();
}

void WorldMaterialBase::PrepareMesh(const Mesh* mesh) 
{
	glBindVertexArray(mesh->GetID());
	m_boundMeshTris = mesh->GetTriangleCount();
}

void WorldMaterialBase::RenderInstance(Transform* transform)
{
	m_shader->SetUniformMat4(m_uniformObjectToWorld, transform == nullptr ? mat4(1.0f) : transform->GetTransformMatrix());

	glDrawElements(GL_TRIANGLES, m_boundMeshTris, GL_UNSIGNED_INT, nullptr);

}