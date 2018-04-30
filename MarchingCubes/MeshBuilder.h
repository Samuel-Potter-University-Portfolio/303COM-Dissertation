#pragma once
#include "Common.h"
#include <vector>
#include <unordered_map>


/**
* Efficiently construct mesh data to finally be turned into a mesh
*/
class MeshBuilderMinimal
{
	///
	/// The actual settings to turn into a mesh
	///
private:
	bool bIsDynamic = false;
	std::vector<vec3> m_vertices;
	std::vector<vec3> m_normals;
	std::vector<uint32> m_indices;
	std::unordered_map<vec3, uint32, vec3_KeyFuncs, vec3_KeyFuncs> m_indexLookup;

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
		m_indices.push_back(a);
		m_indices.push_back(b);
		m_indices.push_back(c);
	}

	/**
	* Builds a mesh from the stored data
	* @param target			The mesh to upload the data to
	*/
	void BuildMesh(class Mesh* target) const;


	/**
	* Simply the mesh by performing edge-collapse
	* @param count			How many triangles to remove
	*/
	void PerformEdgeCollapseReduction(const uint32& count);

	/** Should the resulting mesh be consider for dynamically uploading data*/
	inline void MarkDynamic() { bIsDynamic = true; }

	inline uint32 GetIndexCount() const { return m_indices.size(); }
};