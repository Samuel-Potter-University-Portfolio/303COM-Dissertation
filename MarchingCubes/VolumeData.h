#pragma once
#include "Common.h"


/**
* Inteface for storing and accessing volumetric data
*/
class IVolumeData
{
public:
	/**
	* Initialize this volume with the given settings
	* @param width				The size of the data on the x axis
	* @param height				The size of the data on the y axis
	* @param depth				The size of the data on the z axis
	* @param scale				The scale to use for the data
	* @param defaultValue		The default value to set each voxel to
	*/
	virtual void Init(uint32 width, uint32 height, uint32 depth, vec3 scale, float defaultValue) = 0;

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
	* Load volume data from a Pvm
	* @param file				The URL of the file to load
	*/
	virtual bool LoadFromPvmFile(const char* file);
};


/**
* Default volume storage
*/
class VolumeData : public IVolumeData
{
private:
	///
	/// General Vars
	///
	float* m_data = nullptr;
	vec3 m_scale = vec3(1, 1, 1);
	uint32 m_width;
	uint32 m_height;
	uint32 m_depth;

public:
	~VolumeData();

	virtual void Init(uint32 width, uint32 height, uint32 depth, vec3 scale, float defaultValue) override;

	virtual void Set(uint32 x, uint32 y, uint32 z, float value) override;

	virtual float Get(uint32 x, uint32 y, uint32 z) override;

	///
	/// Getters & Setters
	///
private:
	inline uint32 GetIndex(uint32 x, uint32 y, uint32 z) const { return x + m_width * (y + m_height * z); }
public:
	inline uint32 GetWidth() const { return m_width; }
	inline uint32 GetHeight() const { return m_height; }
	inline uint32 GetDepth() const { return m_depth; }

	inline vec3 GetScale() const { return m_scale; }
};

