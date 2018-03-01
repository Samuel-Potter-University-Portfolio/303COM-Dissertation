#pragma once
#include "Common.h"

#include <vector>


/**
* Lets user load data easily into VBOs
* Stores data in the following format
* -0:	Vertices (3 floats)
* -1:	Normals (3 floats)
* -2:	Colours (4 floats)
* -3:	UVs (2 floats)
* -3+:	Reserved for UVs, but can be used for custom data
*/
class Mesh
{
private:
	///
	/// IDs
	///
	uint32 m_id = 0;
	uint32 m_triangleCount = 0;

	uint32 m_triId = 0;
	uint32 m_bufferId[16]{ 0 };

	///
	/// Mesh Settings
	///
	bool bIsDynamic = false;

public:
	Mesh();
	~Mesh();

	/**
	* Set all of the triangles indices for this mesh
	* @param trangles		List of the indices for drawing a triangle
	*/
	void SetTriangles(std::vector<uint32>& triangles);

	/**
	* Store vertices into the correct buffer
	* @param vertices		The vertex information to store
	*/
	inline void SetVertices(std::vector<vec3>& vertices) { SetBufferData(0, vertices.data(), vertices.size() * sizeof(vec3), 3, false); }

	/**
	* Store normals into the correct buffer
	* @param normals		The vertex information to store
	*/
	inline void SetNormals(std::vector<vec3>& normals) { SetBufferData(1, normals.data(), normals.size() * sizeof(vec3), 3, false); }

	/**
	* Store colours into the correct buffer
	* @param colours		The vertex information to store
	*/
	inline void SetColours(std::vector<vec4>& colours) { SetBufferData(2, colours.data(), colours.size() * sizeof(vec4), 4, false); }

	/**
	* Store uvs into the correct buffer
	* @param uvs			The vertex information to store
	* @param channel		What channel to store the uv information in (0 - 10)
	*/
	inline void SetUVs(std::vector<vec2>& uvs, const uint32& channel = 0) { SetBufferData(3 + channel, uvs.data(), uvs.size() * sizeof(vec2), 2, false); }


private:
	/**
	* Fills a specific buffer with the given data
	* @param index			The index of the buffer to store the data in
	* @param data			The pointer to the data to store
	* @param size			The size in bytes of the data to store
	* @param width			The size (width) of each element (e.g. vec3 = width 3)
	* @param normalized		Has the data being passed through been normalized
	*/
	void SetBufferData(const uint32& index, const void* data, const uint32& size, const uint32& width, const bool& normalized);


	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_id; }
	inline uint32 GetTriangleCount() const { return m_triangleCount; }

	/// Stores data into a faster to write place
	inline void MarkDynamic() { bIsDynamic = true; }
};

