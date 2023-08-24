#include "game.h"

#include <core/Module.h>
#include <core/program.h>

#include <game/GameApplication.h>

using namespace yae;

void initModule(yae::Program* _program, yae::Module* _module)
{
	if (_program->findModule("editor") == nullptr)
	{
		GameApplication* gameApplication = defaultAllocator().create<GameApplication>();
		_module->userData = gameApplication;

		gameApplication->start();
	}
}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{
	if (_module->userData != nullptr)
	{
		GameApplication* gameApplication = (GameApplication*)_module->userData;

		gameApplication->stop();

		defaultAllocator().destroy(gameApplication);
		_module->userData = nullptr;
	}
}

void onModuleReloaded(yae::Program* _program, yae::Module* _module)
{

}

void startProgram(yae::Program* _program, yae::Module* _module)
{

}

void stopProgram(yae::Program* _program, yae::Module* _module)
{

}

void getDependencies(const char*** _outModuleNames, int* _outModuleCount)
{
	static const char* s_dependencies[] = {
		"yae",
	};
	*_outModuleNames = s_dependencies;
	*_outModuleCount = countof(s_dependencies); 
}
