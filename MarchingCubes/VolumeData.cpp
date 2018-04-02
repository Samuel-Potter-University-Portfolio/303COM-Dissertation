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
	Init(uvec3(width, height, depth), scale, 0.0f);

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

bool IVolumeData::Raycast(const Ray& ray, VoxelHitInfo& hit, float maxDistance)
{
	// Source: https://gist.github.com/yamamushi/5823518
	// Bresenham Line Algorithm 3D

	ivec3 a(
		round(ray.GetOrigin().x),
		round(ray.GetOrigin().y),
		round(ray.GetOrigin().z)
	);
	ivec3 b(
		round(ray.GetOrigin().x + ray.GetDirection().x * maxDistance),
		round(ray.GetOrigin().y + ray.GetDirection().y * maxDistance),
		round(ray.GetOrigin().z + ray.GetDirection().z * maxDistance)
	);

	ivec3 point = a;
	ivec3 lastPoint = a;
	float lastValue = 0.0f;
	ivec3 delta = b - a;
	ivec3 lmn = abs(delta);

	ivec3 inc(
		(delta.x < 0) ? -1 : 1,
		(delta.y < 0) ? -1 : 1,
		(delta.z < 0) ? -1 : 1
	);
	ivec3 delta2(
		lmn.x << 1,
		lmn.y << 1,
		lmn.z << 1
	);

	int err_1;
	int err_2;
	
#define CHECK_OUTPUT \
	if(point.x >= 0 && point.x < GetResolution().x && point.y >= 0 && point.y < GetResolution().y && point.z >= 0 && point.z < GetResolution().z) \
	{ \
		float value = Get(point.x, point.y, point.z); \
		if (value >= GetIsoLevel()) \
		{ \
			hit.coord = point; \
			hit.surface = lastPoint; \
			hit.value = value; \
			hit.surfaceValue = lastValue; \
			return true; \
		} \
		lastPoint = point; \
		lastValue = value; \
	}


	if ((lmn.x >= lmn.y) && (lmn.x >= lmn.z))
	{
		err_1 = delta2.y - 1;
		err_2 = delta2.z - 1;
		for (int i = 0; i < lmn.x; ++i)
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.y += inc.y;
				err_1 -= delta2.x;
			}
			if (err_2 > 0)
			{
				point.z += inc.z;
				err_2 -= delta2.x;
			}
			err_1 += delta2.y;
			err_2 += delta2.z;
			point.x += inc.x;
		}
	}
	else if ((lmn.y >= 1) && (lmn.y >= lmn.z))
	{
		err_1 = delta2.x - lmn.y;
		err_2 = delta2.z - lmn.y;
		for (int i = 0; i < lmn.y; ++i)
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.x += inc.x;
				err_1 -= delta2.y;
			}
			if (err_2 > 0)
			{
				point.z += inc.z;
				err_2 -= delta2.y;
			}
			err_1 += delta2.x;
			err_2 += delta2.z;
			point.y += inc.y;
		}
	}
	else
	{
		err_1 = delta2.y - lmn.z;
		err_2 = delta2.x - lmn.z;
		for (int i = 0; i < lmn.z; ++i)
		{
			CHECK_OUTPUT;
			if (err_1 > 0)
			{
				point.y += inc.y;
				err_1 -= delta2.z;
			}
			if (err_2 > 0)
			{
				point.x += inc.x;
				err_2 -= delta2.z;
			}
			err_1 += delta2.y;
			err_2 += delta2.x;
			point.z += inc.z;
		}
	}

	return false;
}