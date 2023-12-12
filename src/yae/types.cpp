#include "types.h"

#include <core/Program.h>
#include <core/Program.h>
#include <core/Module.h>

#include <yae/Application.h>
#include <yae/Engine.h>

namespace yae {

Engine& engine()
{
	Engine* enginePtr = Engine::Instance();
	YAE_ASSERT(enginePtr != nullptr);
	return *enginePtr;
}

ResourceManager& resourceManager()
{
	return app().resourceManager();
}

Application& app()
{
	Application* application = engine().currentApplication();
	YAE_ASSERT(application != nullptr);
	return *application;
}

InputSystem& input()
{
	return app().input();
}

Renderer& renderer()
{
	return app().renderer();
}

} // namespace yae
