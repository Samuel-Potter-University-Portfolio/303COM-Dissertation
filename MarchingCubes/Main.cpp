#include "Engine.h"

#include "Level.h"
#include "Logger.h"

#include "SkyBox.h"
#include "SpectatorController.h"
#include "VoxelVolume.h"
#include "ChunkedVolume.h"


int main(int argc, char** argv)
{
	EngineInit settings;
	settings.Title = "Marching Cubes";
	Engine engine(settings);

	Level* level = new Level;

	level->AddObject(new SpectatorController);
	level->AddObject(new SkyBox);
	level->AddObject(new ChunkedVolume);
	engine.SetLevel(level);
	engine.LaunchMainLoop();
	return 0;
}
