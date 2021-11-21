#pragma once

#include <yae/types.h>
#include <yae/filesystem.h>
#include <yae/containers/Array.h>

namespace yae {

class Allocator;
class Application;
class ResourceManager;
class Logger;
class Profiler;

class YAELIB_API Program
{
public:
	Program(Allocator* _defaultAllocator, Allocator* _scratchAllocator, Allocator* _toolAllocator);
	~Program();

	void init(char** _args, int _argCount);
	void shutdown();

	void run();

	void registerApplication(Application* _application);
	void unregisterApplication(Application* _application);

	const Path& getExePath() const;

	Application& currentApplication();
	Allocator& defaultAllocator();
	Allocator& scratchAllocator();
	Allocator& toolAllocator();
	ResourceManager& resourceManager();
	Logger& logger();
	Profiler& profiler();

// private
	Allocator* m_defaultAllocator = nullptr;
	Allocator* m_scratchAllocator = nullptr;
	Allocator* m_toolAllocator = nullptr;
	Logger* m_logger = nullptr;
	ResourceManager* m_resourceManager = nullptr;
	Profiler* m_profiler = nullptr;

	Array<Application*> m_applications;
	Application* m_currentApplication = nullptr;

	int m_argCount = 0;
	char** m_args = nullptr;
	Path m_exePath;

	static Program* s_programInstance;
};

}
