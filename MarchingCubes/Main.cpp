#include "Engine.h"

#include "Level.h"
#include "Logger.h"

#include "SkyBox.h"
#include "SpectatorController.h"

#include "DefaultVolume.h"
#include "ChunkedVolume.h"
#include "OctreeVolume.h"
#include "OctreeRepVolume.h"
#include "LayeredVolume.h"


/*
// Debug
#include <set>
uint8 RotateIndices(uint32 x, uint32 y, uint32 z, uint8 caseIndex)
{
	uint8 newCase = caseIndex;

	while (x != 0)
	{
		newCase = 0;

		if (caseIndex & 1) newCase |= 8;
		if (caseIndex & 2) newCase |= 4;
		if (caseIndex & 4) newCase |= 64;
		if (caseIndex & 8) newCase |= 128;
		if (caseIndex & 16) newCase |= 1;
		if (caseIndex & 32) newCase |= 2;
		if (caseIndex & 64) newCase |= 32;
		if (caseIndex & 128) newCase |= 16;

		caseIndex = newCase;
		x--;
	}
	while (y != 0)
	{
		newCase = 0;

		if (caseIndex & 1) newCase |= 2;
		if (caseIndex & 2) newCase |= 4;
		if (caseIndex & 4) newCase |= 8;
		if (caseIndex & 8) newCase |= 1;
		if (caseIndex & 16) newCase |= 32;
		if (caseIndex & 32) newCase |= 64;
		if (caseIndex & 64) newCase |= 128;
		if (caseIndex & 128) newCase |= 16;

		caseIndex = newCase;
		y--;
	}
	while (z != 0)
	{
		newCase = 0;

		if (caseIndex & 1) newCase |= 2;
		if (caseIndex & 2) newCase |= 32;
		if (caseIndex & 4) newCase |= 64;
		if (caseIndex & 8) newCase |= 4;
		if (caseIndex & 16) newCase |= 1;
		if (caseIndex & 32) newCase |= 16;
		if (caseIndex & 64) newCase |= 128;
		if (caseIndex & 128) newCase |= 8;

		caseIndex = newCase;
		z--;
	}

	return newCase;
}

uint8 MirrorIndices(bool x, bool y, bool z, uint8 caseIndex)
{
	uint8 newCase = 0;

	if (x)
	{
		newCase = 0;
		if (caseIndex & 1) newCase |= 2;
		if (caseIndex & 2) newCase |= 1;
		if (caseIndex & 4) newCase |= 8;
		if (caseIndex & 8) newCase |= 4;
		if (caseIndex & 16) newCase |= 32;
		if (caseIndex & 32) newCase |= 16;
		if (caseIndex & 64) newCase |= 128;
		if (caseIndex & 128) newCase |= 64;

		caseIndex = newCase;
	}
	if (y)
	{
		newCase = 0;
		if (caseIndex & 1) newCase |= 16;
		if (caseIndex & 2) newCase |= 32;
		if (caseIndex & 4) newCase |= 64;
		if (caseIndex & 8) newCase |= 128;
		if (caseIndex & 16) newCase |= 1;
		if (caseIndex & 32) newCase |= 2;
		if (caseIndex & 64) newCase |= 4;
		if (caseIndex & 128) newCase |= 8;

		caseIndex = newCase;
	}
	if (z)
	{
		newCase = 0;
		if (caseIndex & 1) newCase |= 8;
		if (caseIndex & 2) newCase |= 4;
		if (caseIndex & 4) newCase |= 2;
		if (caseIndex & 8) newCase |= 1;
		if (caseIndex & 16) newCase |= 128;
		if (caseIndex & 32) newCase |= 64;
		if (caseIndex & 64) newCase |= 32;
		if (caseIndex & 128) newCase |= 16;

		caseIndex = newCase;
	}

	return newCase;
}

#include <iostream>
void GenSimpleCases() 
{
	std::set<uint8> baseCases;

	// Case 1,2,5,8,9,11,14
	baseCases.emplace(1);
	baseCases.emplace(1 | 2);
	baseCases.emplace(2 | 4 | 8);
	baseCases.emplace(1 | 2 | 4 | 8);
	baseCases.emplace(1 | 8 | 4 | 128);
	baseCases.emplace(1 | 8 | 4 | 64);
	baseCases.emplace(8 | 4 | 2 | 128);

	std::set<uint8> totalCases(baseCases);

	for (const uint8& caseIndex : baseCases)
	{
		for (uint32 xr = 0; xr < 7; xr++)
			for (uint32 yr = 0; yr < 7; yr++)
				for (uint32 zr = 0; zr < 7; zr++)
				{
					uint8 newCase = RotateIndices(xr, yr, zr, caseIndex);
					totalCases.emplace(newCase);
					totalCases.emplace(~newCase);
					uint8 newInCase = RotateIndices(xr, yr, zr, ~caseIndex);
					totalCases.emplace(newInCase);
					totalCases.emplace(~newInCase);
				}
	}

	for (uint8 caseIndex : totalCases)
		std::cout << (uint32)caseIndex << ", ";
	return;
}
*/

int main(int argc, char** argv)
{
	const uint32 size = 8;
	const uint32 halfSize = size / 2;
	const uint32 quatSize = size / 4;

	const uint32 start = (pow(8, 3) - 1) / 7;

	for (uint32 z = 0; z < size; ++z)
	for (uint32 y = 0; y < size; ++y)
		for (uint32 x = 0; x < size; ++x)
		{
			//m_startIndex + x + m_layerResolution *(y + m_layerResolution * z);
			int v = start + x % 2 + (x / 2) * 8 +
				(y % 2) * 2 + (y / 2) * 4 * size +
				(z % 2) * 4 + (z / 2) * 2 * size * size;


			uint32 o = (v - start);
			uint32 nx = ((o / 8) % size);
			uint32 ny = 0;
			uint32 nz = ((o % 8)) / 4 + (o / (size * size * 2));

			LOG("(%i,%i,%i) => %i => (%i,%i,%i)", x, y, z, v - start, nx, ny, nz);
		}
	// 585
	return 0;

	EngineInit settings;
	settings.Title = "Marching Cubes";
	Engine engine(settings);
	Level* level = new Level;

	level->AddObject(new SpectatorController);
	level->AddObject(new SkyBox);
	level->AddObject(new LayeredVolume);
	engine.SetLevel(level);
	engine.LaunchMainLoop();
	return 0;
}
