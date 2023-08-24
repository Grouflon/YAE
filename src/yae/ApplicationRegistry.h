#pragma once

#include <core/containers/Array.h>

namespace yae {

class Application;

class ApplicationRegistry
{
public:
	ApplicationRegistry();
	~ApplicationRegistry();

	static void StartApplication(Application* _application);
	static void StopApplication(Application* _application);

	static void Update();

	static Application* CurrentApplication();

//private:
	DataArray<Application*> m_applications;
	DataArray<Application*> m_applicationStack;
};

} // namespace yae
