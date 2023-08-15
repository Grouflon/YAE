#include "editor_module.h"

#include <yae/Application.h>

#include <editor/Editor.h>

using namespace yae;

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{

}

void onModuleUnloaded(yae::Program* _program, yae::Module* _module)
{

}

void initModule(yae::Program* _program, yae::Module* _module)
{

}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{

}

void initApplication(yae::Application* _application, const char** _args, int _argCount)
{
	editor::Editor* editor = toolAllocator().create<editor::Editor>();
	_application->setUserData("Editor", editor);

	editor->init();
}

void shutdownApplication(yae::Application* _application)
{
	editor::Editor* editor = (editor::Editor*)_application->getUserData("Editor");

	editor->shutdown();

	_application->setUserData("Editor", nullptr);
	toolAllocator().destroy(editor);	
}

void onApplicationModuleReloaded(yae::Application* _application, yae::Module* _module)
{
	editor::Editor* editor = (editor::Editor*)_application->getUserData("Editor");

	editor->reload();
}

void updateApplication(yae::Application* _application, float _dt)
{
	editor::Editor* editor = (editor::Editor*)_application->getUserData("Editor");

	editor->update(_dt);
}

bool onSerializeApplicationSettings(Application* _application, Serializer* _serializer)
{
	editor::Editor* editor = (editor::Editor*)_application->getUserData("Editor");

	return editor->serialize(_serializer);
}
