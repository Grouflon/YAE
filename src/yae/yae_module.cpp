#include "yae_module.h"

#include <core/memory.h>
#include <core/Program.h>
#include <core/Module.h>

#include <yae/Application.h>
#include <yae/Engine.h>

using namespace yae;

// Module Interface

void initModule(Program* _program, Module* _module)
{
	Engine* engine_ = defaultAllocator().create<Engine>(); 
	_module->userData = engine_;
	engine_->init();
}

void shutdownModule(Program* _program, Module* _module)
{
	engine().shutdown();
	defaultAllocator().destroy((Engine*)_module->userData);
	_module->userData = nullptr;
}

void beforeModuleReload(Program* _program, Module* _module)
{
	Engine* engine = (Engine*)_module->userData;
	engine->beforeReload();
}

void afterModuleReload(Program* _program, Module* _module)
{
	Engine* engine = (Engine*)_module->userData;
	engine->afterReload();
}

void startProgram(Program* _program, Module* _module)
{
}

void stopProgram(Program* _program, Module* _module)
{
}

void updateProgram(Program* _program, Module* _module)
{
	engine().update();
}

bool serializeSettings(yae::Serializer& _serializer)
{
	return engine().serializeSettings(_serializer);
}

