#include "VolumeData.h"

#include "Logger.h"
#include <fstream>


bool IVolumeData::LoadFromPvmFile(const char* file)
{
	std::ifstream stream(file, std::ifstream::in);
	if (!stream.good())
	{
		LOG_ERROR("Cannot locate file '%s'", file);
		return false;
	}


	std::string format;
	std::getline(stream, format);

	if (format == "DDS v3d")
	{

		return true;
	}
	else 
	{
		LOG_ERROR("Unknown PVM format '%s' for '%s'", format.c_str(), file);
		return false;
	}
}


VolumeData::~VolumeData() 
{
	if (m_data != nullptr)
		delete[] m_data;
}

void VolumeData::Init(uint32 width, uint32 height, uint32 depth, float defaultValue) 
{
	m_data = new float[width * height * depth]{ defaultValue };
	m_width = width;
	m_height = height;
	m_depth = depth;
}

void VolumeData::Set(uint32 x, uint32 y, uint32 z, float value) 
{
	m_data[GetIndex(x, y, z)] = value;
}

float VolumeData::Get(uint32 x, uint32 y, uint32 z) 
{
	return m_data[GetIndex(x, y, z)];
}