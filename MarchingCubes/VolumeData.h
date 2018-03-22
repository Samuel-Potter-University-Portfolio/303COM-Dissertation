#pragma once
#include "Common.h"
#include "Ray.h"


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
class IVolumeData
{
public:
	/**
	* Initialize this volume with the given settings
	* @param resolution			The (initial) resolution of the data 
	* @param scale				The scale to use for the data
	* @param defaultValue		The default value to set each voxel to
	*/
	virtual void Init(const uvec3& resolution, const vec3& scale, float defaultValue) = 0;


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
