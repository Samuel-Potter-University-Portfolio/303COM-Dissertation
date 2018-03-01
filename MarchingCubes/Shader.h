#pragma once
#include "Common.h"


/**
* Loads several GLSL shaders and links them
*/
class Shader
{
private:
	///
	/// Shader ids
	///
	uint32 m_programId;

	uint32 m_vertexId;
	uint32 m_fragmentId;

	///
	/// Shader settings
	///
	bool bEnableCullFace = true;
	bool bCullBackFace = true;
	bool bEnableDepthTest = true;

public:
	Shader();
	virtual ~Shader();

	/**
	* Bind all of the settings required to use this shader
	*/
	virtual void Bind();


	///
	/// Loading/Building shader
	///
public:
	/**
	* Adds a vertex shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadVertexShaderFromMemory(const string& code);
	/**
	* Adds a fragment shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadFragmentShaderFromMemory(const string& code);

	/**
	* Adds a vertex shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadVertexShaderFromFile(const string& file);
	/**
	* Adds a fragment shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadFragmentShaderFromFile(const string& file);

	/**
	* Attempt to link all subshaders and store them in a program
	* @returns If succesful or not
	*/
	bool LinkShader();


	///
	/// Uniforms fetching and setting
	///
public:
	/**
	* Get a uniform by name
	* @param name		The name (in source) for the uniform
	* @returns The id of the uniform
	*/
	uint32 GetUniform(const string& name) const;

	/**
	* Load a float into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformFloat(const uint32& id, const float& value);
	/**
	* Load a int into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformInt(const uint32& id, const int32& value);
	/**
	* Load a int into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformUInt(const uint32& id, const uint32& value);

	/**
	* Load a vec2 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformVec2(const uint32& id, const vec2& value);
	/**
	* Load a vec3 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformVec3(const uint32& id, const vec3& value);
	/**
	* Load a vec4 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformVec4(const uint32& id, const vec4& value);

	/**
	* Load a mat2 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformMat2(const uint32& id, const mat2& value);
	/**
	* Load a mat3 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformMat3(const uint32& id, const mat3& value);
	/**
	* Load a mat4 into this unform id
	* @param id			The id of the uniform (fetched from GetUniform)
	* @param value		The value to load into the location
	*/
	void SetUniformMat4(const uint32& id, const mat4& value);


	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_programId; }

	inline void SetCullFace(const bool& value) { bEnableCullFace = value; }
	inline bool GetCullFace() const { return bEnableCullFace; }

	inline void SetCullBackFace(const bool& value) { bCullBackFace = value; }
	inline bool GetCullBackFace() const { return bCullBackFace; }

	inline void SetDepthTest(const bool& value) { bEnableDepthTest = value; }
	inline bool GetDepthTest() const { return bEnableDepthTest; }
};

