#include "Engine.h"

#include "Level.h"
#include "Logger.h"

#include "SkyBox.h"
#include "SpectatorController.h"

#include "DefaultVolume.h"
#include "ChunkedVolume.h"
#include "OctreeVolume.h"
#include "OctreeRepVolume.h"


// Debug
#include <set>
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
}

int main(int argc, char** argv)
{




	EngineInit settings;
	settings.Title = "Marching Cubes";
	Engine engine(settings);
	Level* level = new Level;

	level->AddObject(new SpectatorController);
	level->AddObject(new SkyBox);
	level->AddObject(new OctreeRepVolume);
	engine.SetLevel(level);
	engine.LaunchMainLoop();
	return 0;
}
