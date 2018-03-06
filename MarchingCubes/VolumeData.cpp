#include "VolumeData.h"

#include "Logger.h"
#include "PVM/ddsbase.h"

#include <fstream>


bool IVolumeData::LoadFromPvmFile(const char* file)
{
	uint8* volume;
	uint32 width, height, depth, components;
	vec3 scale;

	if ((volume = readPVMvolume(file, &width, &height, &depth, &components, &scale.x, &scale.y, &scale.z)) == nullptr)
	{
		LOG_ERROR("Failed to load '%s'", file);
		return false;
	}

	if (components > 2)
		LOG_WARNING("Cannot currently support %i components for volumes '%s'", components, file);


	// Convert binary data into float data
	Init(width, height, depth, scale, 0.0f);

	uint8* data = volume;
	for (uint32 z = 0; z < depth; ++z)
		for (uint32 y = 0; y < height; ++y)
			for (uint32 x = 0; x < width; ++x)
			{

				if (components == 1)
				{
					Set(x, y, z, (float)(*data) / 255.0f);
					data++;
				}
				else if (components == 2)
				{
					uint16* d = (uint16*)(data);
					Set(x, y, z, (float)(*d) / 65535);
					data += 2;
				}
			}


	free(volume);
	return true;
}


VolumeData::~VolumeData() 
{
	if (m_data != nullptr)
		delete[] m_data;
}

void VolumeData::Init(uint32 width, uint32 height, uint32 depth, vec3 scale, float defaultValue) 
{
	m_data = new float[width * height * depth]{ defaultValue };
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_scale = scale;
}

void VolumeData::Set(uint32 x, uint32 y, uint32 z, float value) 
{
	m_data[GetIndex(x, y, z)] = value;
}

float VolumeData::Get(uint32 x, uint32 y, uint32 z) 
{
	return m_data[GetIndex(x, y, z)];
}