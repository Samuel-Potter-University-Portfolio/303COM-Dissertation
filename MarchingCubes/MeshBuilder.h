#pragma once
#include "Common.h"
#include <vector>
#include <unordered_map>

/**
* Efficiently construct mesh data to finally be turned into a mesh
*/
class MeshBuilderFull
{
private:
	/**
	* Entry about a single vertex
	*/
	struct VertexEntry 
	{
		vec3 position;
		vec3 normal;
		vec4 colour;
		vec2 uv;
	};

	/**
	* Required functions to use VertexEntry as key
	*/
	struct Vertex_KeyFuncs
	{
		inline size_t operator()(const VertexEntry& v)const
		{
			return 
				std::hash<int>()(v.position.x) ^ std::hash<int>()(v.position.y) ^ std::hash<int>()(v.position.z) ^
				std::hash<int>()(v.normal.x) ^ std::hash<int>()(v.normal.y) ^ std::hash<int>()(v.normal.z) ^
				std::hash<int>()(v.colour.r) ^ std::hash<int>()(v.colour.g) ^ std::hash<int>()(v.colour.b) ^ std::hash<int>()(v.colour.a) ^
				std::hash<int>()(v.uv.x) ^ std::hash<int>()(v.uv.y)
				;
		}

		inline bool operator()(const VertexEntry& a, const VertexEntry& b)const
		{
			return a.position == b.position && a.normal == b.normal && a.colour == b.colour && a.uv == b.uv;
		}
	};


	///
	/// The actual settings to turn into a mesh
	///
private:
	bool bIsDynamic = false;
	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;
	std::vector<vec4> m_colours;
	std::vector<vec2> m_uvs;
	std::vector<uint32> m_triangles;

	std::unordered_map<VertexEntry, uint32, Vertex_KeyFuncs, Vertex_KeyFuncs> m_vertexLookup;

public:

	/**
	* Add this vertex to the recipe
	* @param vertex			The position of the vertex
	* @param normal			The normal of the vertex
	* @param colour			The colour of the vertex
	* @param uv				The uv of the vertex
	* @returns The index of the vertex
	*/
	uint32 AddVertex(const vec3& vertex, const vec3& normal = vec3(0, 1, 0), const vec4& colour = vec4(1, 1, 1, 1), const vec2& uv = vec2(0, 0));

	/**
	* Connects these 3 vertices to form a triangle
	* @param a,b,c			The index of the vertices to connect (In anticlockwise order)
	*/
	inline void AddTriangle(const uint32& a, const uint32& b, const uint32& c) 
	{
		m_triangles.push_back(a);
		m_triangles.push_back(b);
		m_triangles.push_back(c);
	}

	/**
	* Smooth out all the normals in the mesh
	* (Will overwrite all previously set normals)
	*/
	void SmoothNormals();

	/**
	* Builds a mesh from the stored data
	* @param target			The mesh to upload the data to
	*/
	void BuildMesh(class Mesh* target) const;


	/** Should the resulting mesh be consider for dynamically uploading data*/
	inline bool MarkDynamic() { bIsDynamic = true; }
};


/**
* Efficiently construct mesh data to finally be turned into a mesh
*/
class MeshBuilderMinimal
{
private:
	/**
	* Entry about a single vertex
	*/
	struct VertexEntry
	{
		vec3 position;
		vec3 normal;
	};

	/**
	* Required functions to use VertexEntry as key
	*/
	struct Vertex_KeyFuncs
	{
		inline size_t operator()(const VertexEntry& v)const
		{
			return
				std::hash<int>()(v.position.x) ^ std::hash<int>()(v.position.y) ^ std::hash<int>()(v.position.z) ^
				std::hash<int>()(v.normal.x) ^ std::hash<int>()(v.normal.y) ^ std::hash<int>()(v.normal.z)
				;
		}

		inline bool operator()(const VertexEntry& a, const VertexEntry& b)const
		{
			bool pos = a.position == b.position;
			bool nor = a.normal == b.normal;
			return a.position == b.position && a.normal == b.normal;
		}
	};


	///
	/// The actual settings to turn into a mesh
	///
private:
	bool bIsDynamic = false;
	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;
	std::vector<uint32> m_triangles;

	std::unordered_map<VertexEntry, uint32, Vertex_KeyFuncs, Vertex_KeyFuncs> m_vertexLookup;

public:

	/**
	* Add this vertex to the recipe
	* @param vertex			The position of the vertex
	* @param normal			The normal of the vertex
	* @returns The index of the vertex
	*/
	uint32 AddVertex(const vec3& vertex, const vec3& normal = vec3(0, 1, 0));

	/**
	* Connects these 3 vertices to form a triangle
	* @param a,b,c			The index of the vertices to connect (In anticlockwise order)
	*/
	inline void AddTriangle(const uint32& a, const uint32& b, const uint32& c)
	{
		m_triangles.push_back(a);
		m_triangles.push_back(b);
		m_triangles.push_back(c);
	}

	/**
	* Smooth out all the normals in the mesh
	* (Will overwrite all previously set normals)
	*/
	void SmoothNormals();

	/**
	* Builds a mesh from the stored data
	* @param target			The mesh to upload the data to
	*/
	void BuildMesh(class Mesh* target) const;


	/** Should the resulting mesh be consider for dynamically uploading data*/
	inline void MarkDynamic() { bIsDynamic = true; }
};