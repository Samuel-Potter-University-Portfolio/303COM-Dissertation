#pragma once
#include "Common.h"
#include "Ray.h"
#include <vector>


/// The value that every voxel will be set to by defaul
#ifndef DEFAULT_VALUE
#define DEFAULT_VALUE 0.0f
#endif


/// The value that should be assumed if trying to access invalid voxel, while building
#ifndef UNKNOWN_BUILD_VALUE
#define UNKNOWN_BUILD_VALUE 0.0f
#endif


/**
* Partial data about a voxel sub-sections
*/
struct VoxelPartialMeshData
{
	/**
	* Information about a single triangle
	*/
	struct TriData
	{
		vec3 a;
		vec3 b;
		vec3 c;
		vec3 weightedNormal; // weighted, so normals can be calculated more fairly later on

		TriData(const vec3& a, const vec3& b, const vec3& c, const vec3& weightedNormal)
			: a(a), b(b), c(c), weightedNormal(weightedNormal)
		{}
	};

public:
	std::vector<TriData> triangles;
	bool isStale;

	inline void AddTriangle(const vec3& a, const vec3& b, const vec3& c, const vec3& weightedNormal) { triangles.emplace_back(a, b, c, weightedNormal); }
	inline void Clear() { triangles.clear(); isStale = false; }
};



/**
* Structure used when attempting to raycast against a voxel volume
*/
struct VoxelHitInfo
{
	float value;
	float surfaceValue;
	ivec3 coord;
	ivec3 surface;
};


/**
* Inteface for storing and accessing volumetric data
*/
class IVoxelVolume
{
public:
	/**
	* Initialize this volume with the given settings
	* @param resolution			The (initial) resolution of the data 
	* @param scale				The scale to use for the data
	*/
	virtual void Init(const uvec3& resolution, const vec3& scale) = 0;


	/**
	* Set the value of a specific voxel
	* @param x,y,z				The coordinate of the voxel to change
	* @param value				The value to set the voxel to
	*/
	virtual void Set(uint32 x, uint32 y, uint32 z, float value) = 0;
	/**
	* Retrieve the value of a specific voxel
	* @param x,y,z				The coordinate of the voxel to get
	* @returns The value at the voxel
	*/
	virtual float Get(uint32 x, uint32 y, uint32 z) = 0;

	/**
	* Get the resolution of the data currently stored
	* @return The resolution for x,y,z
	*/
	virtual uvec3 GetResolution() const = 0;
	/**
	* Does this volume data support dynamic resolution
	* @return True if resolution can change
	*/
	virtual bool SupportsDynamicResolution() const = 0;

	/**
	* Get the iso level that the volume is currently being viewed at
	* @return The iso level of the data [0.0-1.0]
	*/
	virtual float GetIsoLevel() const = 0;


	/**
	* Fetch information about this voxel
	* @param ray				The ray to cast at the volume
	* @param outHit				Where to store the information about the voxel
	* @param isoLevel			The isoLevel to compare against
	* @param maxDistance		The maximum distance of the ray
	*/
	virtual bool Raycast(const Ray& ray, VoxelHitInfo& outHit, float maxDistance = 100.0f);


	/**
	* Load volume data from a Pvm
	* @param file				The URL of the file to load
	*/
	virtual bool LoadFromPvmFile(const char* file);
};
