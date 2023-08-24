#include "editor_module.h"

#include <core/Module.h>

#include <yae/Application.h>

#include <editor/EditorApplication.h>

using namespace yae;

void initModule(yae::Program* _program, yae::Module* _module)
{
	yae::editor::EditorApplication* application = toolAllocator().create<yae::editor::EditorApplication>("yae Editor | " YAE_CONFIG, 800u, 600u);
	_module->userData = application;

	application->start();
}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{
	yae::editor::EditorApplication* application = (yae::editor::EditorApplication*)_module->userData;
	application->stop();

	toolAllocator().destroy(application);
	_module->userData = nullptr;
}

void onModuleReloaded(yae::Program* _program, yae::Module* _module)
{
	yae::editor::EditorApplication* application = (yae::editor::EditorApplication*)_module->userData;
	application->reload();
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
		"game"
	};
	*_outModuleNames = s_dependencies;
	*_outModuleCount = countof(s_dependencies); 
}
