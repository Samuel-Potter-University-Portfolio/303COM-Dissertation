#include "Mesh.h"
#include "Logger.h"

#include <GL\glew.h>


Mesh::Mesh()
{
	glGenVertexArrays(1, &m_id);
}

Mesh::~Mesh()
{
	if (m_id != 0)
		glDeleteVertexArrays(1, &m_id);
}


void Mesh::SetTriangles(std::vector<uint32>& triangles)
{
	glBindVertexArray(m_id);

	if (m_triId == 0)
		glGenBuffers(1, &m_triId);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_triId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(uint32), triangles.data(), bIsDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	m_triangleCount = triangles.size();


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::SetBufferData(const uint32& index, const void* data, const uint32& size, const uint32& width, const bool& normalized)
{
#ifdef _DEBUG
	if (index >= 16)
	{
		LOG_ERROR("Cannot set mesh buffer '%i' as it is over the limit '16'", index);
		return;
	}
#endif

	uint32& id = m_bufferId[index];
	if (id == 0)
		glGenBuffers(1, &id);

	glBindVertexArray(m_id);

	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, bIsDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, width, GL_FLOAT, normalized, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}