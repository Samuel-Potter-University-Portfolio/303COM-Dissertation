#include "Engine.h"

#include "Level.h"
#include "Logger.h"


int main(int argc, char** argv)
{
	EngineInit settings;
	settings.Title = "Marching Cubes";
	Engine engine(settings);

	Level* level = new Level;


	engine.SetLevel(level);
	engine.LaunchMainLoop();
	return 0;
}
