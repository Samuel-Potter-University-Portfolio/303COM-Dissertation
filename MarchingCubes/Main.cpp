#include "Engine.h"
#include "Logger.h"


int main(int argc, char** argv)
{
	EngineInit settings;
	settings.Title = "Marching Cubes";

	Engine engine(settings);
	engine.LaunchMainLoop();
	return 0;
}