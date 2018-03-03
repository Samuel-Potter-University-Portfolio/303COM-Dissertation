#include "Shader.h"
#include "Logger.h"

#include <GL\glew.h>
#include <fstream>


Shader::Shader()
{
}

Shader::~Shader()
{
	if (m_programId != 0)
		glDeleteProgram(m_programId);
}


void Shader::Bind()
{
	glUseProgram(m_programId);


	if (bEnableCullFace)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(bCullBackFace ? GL_BACK : GL_FRONT);
	}
	else
		glDisable(GL_CULL_FACE);


	if (bEnableDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void Shader::Unbind()
{
	glUseProgram(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

bool Shader::LoadVertexShaderFromMemory(const string& code)
{
	m_vertexId = glCreateShader(GL_VERTEX_SHADER);

	const char* str = code.c_str();
	const int32 length = code.size();

	glShaderSource(m_vertexId, 1, &str, &length);
	glCompileShader(m_vertexId);


	// Make sure compile correctly
	int32 result;
	glGetShaderiv(m_vertexId, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_vertexId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_vertexId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader(vert) compile error: '%s'", logMessage);
		delete[] logMessage;
		m_vertexId = 0;
		return false;
	}

	return true;
}

bool Shader::LoadFragmentShaderFromMemory(const string& code)
{
	m_fragmentId = glCreateShader(GL_FRAGMENT_SHADER);

	const char* str = code.c_str();
	const int32 length = code.size();

	glShaderSource(m_fragmentId, 1, &str, &length);
	glCompileShader(m_fragmentId);


	// Make sure compile correctly
	int32 result;
	glGetShaderiv(m_fragmentId, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_fragmentId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_fragmentId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader(frag) compile error: '%s'", logMessage);
		delete[] logMessage;
		m_fragmentId = 0;
		return false;
	}

	return true;
}

bool Shader::LoadVertexShaderFromFile(const string& file)
{
	std::ifstream stream(file);
	if (stream.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		return LoadVertexShaderFromMemory(str);
	}

	LOG_ERROR("Cannot find vertex shader file '%s'", file.c_str());
	return false;
}

bool Shader::LoadFragmentShaderFromFile(const string& file)
{
	std::ifstream stream(file);
	if (stream.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		return LoadFragmentShaderFromMemory(str);
	}

	LOG_ERROR("Cannot find fragment shader file '%s'", file.c_str());
	return false;
}

bool Shader::LinkShader()
{
	m_programId = glCreateProgram();

	glAttachShader(m_programId, m_vertexId);
	glAttachShader(m_programId, m_fragmentId);
	glLinkProgram(m_programId);

	// Make sure link correctly
	int32 result;
	glGetShaderiv(m_programId, GL_LINK_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_programId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader link error: '%s'", logMessage);
		delete[] logMessage;
		m_programId = 0;
		return false;
	}

	return true;
}


uint32 Shader::GetUniform(const string& name) const
{
	return glGetUniformLocation(m_programId, name.c_str());
}

void Shader::SetUniformFloat(const uint32& id, const float& value)
{
	glUniform1f(id, value);
}
void Shader::SetUniformInt(const uint32& id, const int32& value)
{
	glUniform1i(id, value);
}
void Shader::SetUniformUInt(const uint32& id, const uint32& value)
{
	glUniform1ui(id, value);
}

void Shader::SetUniformVec2(const uint32& id, const vec2& value)
{
	glUniform2fv(id, 1, &value[0]);
}
void Shader::SetUniformVec3(const uint32& id, const vec3& value)
{
	glUniform3fv(id, 1, &value[0]);
}
void Shader::SetUniformVec4(const uint32& id, const vec4& value)
{
	glUniform4fv(id, 1, &value[0]);
}

void Shader::SetUniformMat2(const uint32& id, const mat2& value)
{
	glUniformMatrix2fv(id, 1, GL_FALSE, &value[0][0]);
}
void Shader::SetUniformMat3(const uint32& id, const mat3& value)
{
	glUniformMatrix3fv(id, 1, GL_FALSE, &value[0][0]);
}
void Shader::SetUniformMat4(const uint32& id, const mat4& value)
{
	glUniformMatrix4fv(id, 1, GL_FALSE, &value[0][0]);
}