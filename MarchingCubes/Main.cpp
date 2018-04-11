#include "Engine.h"

#include "Level.h"
#include "Logger.h"

#include "SkyBox.h"
#include "SpectatorController.h"

#include "DefaultVolume.h"
#include "ChunkedVolume.h"
#include "OctreeVolume.h"
#include "OctreeRepVolume.h"


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
