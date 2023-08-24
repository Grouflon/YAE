#include "types.h"

#include <core/program.h>
#include <core/Module.h>

#include <yae/ApplicationRegistry.h>
#include <yae/Application.h>

namespace yae {

Application& app()
{
	Application* application = ApplicationRegistry::CurrentApplication();
	YAE_ASSERT(application != nullptr);
	return *application;
}

ResourceManager& resourceManager()
{
	return app().resourceManager();
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
