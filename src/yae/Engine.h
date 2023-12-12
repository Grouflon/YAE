#pragma once

#include <yae/types.h>
#include <core/containers/Array.h>

namespace yae {

class TestSystem;
class FileWatchSystem;

class YAE_API Engine
{
public:
	Engine();
	~Engine();

	void init();
	void shutdown();
	void beforeReload();
	void afterReload();
	void update();

	void startApplication(Application* _application);
	void stopApplication(Application* _application);

	Application* currentApplication();
	FileWatchSystem& fileWatchSystem();

	bool serializeSettings(yae::Serializer& _serializer);

//private:
	DataArray<Application*> m_applications;
	DataArray<Application*> m_applicationStack;

	TestSystem* m_testSystem = nullptr;
	FileWatchSystem* m_fileWatchSystem = nullptr;
	
	bool m_isInitialized = false;

	static Engine* s_engineInstance;
	static Engine* Instance();
};

} // namespace yae
