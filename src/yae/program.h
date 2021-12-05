#pragma once

#include <yae/types.h>
#include <yae/date.h>
#include <yae/containers/Array.h>

// shaderc forward declarations
struct shaderc_compiler;

namespace yae {

class Allocator;
class Application;
class ResourceManager;
class Logger;
class Profiler;

typedef void (*GameFunctionPtr)();

struct GameAPI
{
	void* libraryHandle = nullptr;
	GameFunctionPtr gameInit = nullptr;
	GameFunctionPtr gameUpdate = nullptr;
	GameFunctionPtr gameShutdown = nullptr;
	GameFunctionPtr onLibraryLoaded = nullptr;
	GameFunctionPtr onLibraryUnloaded = nullptr;
};

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

	// Paths & Directories
	const char* getExePath() const;
	const char* getBinDirectory() const;
	const char* getRootDirectory() const;
	const char* getIntermediateDirectory() const;

	// Services getters
	Application& currentApplication();
	Allocator& defaultAllocator();
	Allocator& scratchAllocator();
	Allocator& toolAllocator();
	ResourceManager& resourceManager();
	Logger& logger();
	Profiler& profiler();

	shaderc_compiler* shaderCompiler() { return m_shaderCompiler; }

	// Game API functions
	void initGame();
	void updateGame();
	void shutdownGame();

// private

	void _loadGameAPI(const char* _path);
	void _unloadGameAPI();
	void _copyAndLoadGameAPI(const char* _dllPath, const char* _symbolsPath);
	String _getGameDLLPath() const;
	String _getGameDLLSymbolsPath() const;

	Allocator* m_defaultAllocator = nullptr;
	Allocator* m_scratchAllocator = nullptr;
	Allocator* m_toolAllocator = nullptr;
	Logger* m_logger = nullptr;
	ResourceManager* m_resourceManager = nullptr;
	Profiler* m_profiler = nullptr;
	shaderc_compiler* m_shaderCompiler = nullptr;

	Array<Application*> m_applications;
	Application* m_currentApplication = nullptr;

	int m_argCount = 0;
	char** m_args = nullptr;
	// NOTE: don't forget to initialize those string's allocator in the constructor.
	// The program isn't ready yet so they can't do it themselves.
	String m_exePath;
	String m_binDirectory;
	String m_rootDirectory;
	String m_intermediateDirectory;
	String m_hotReloadDirectory;

	// Game API
	bool m_hotReloadGameAPI = false;
	Date m_gameDLLLastWriteTime = 0;
	GameAPI m_gameAPI;

	static Program* s_programInstance;
};

}
