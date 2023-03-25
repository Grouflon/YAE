#pragma once

#include <yae/types.h>
#include <yae/Date.h>
#include <yae/containers/Array.h>

namespace yae {

class Allocator;
class Application;
class ResourceManager;
class ResourceManager2;
class Logger;
class Profiler;
class Module;

class YAE_API Program
{
public:
	Program();
	~Program();

	void registerModule(const char* _moduleName);

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
	const char* getSettingsDirectory() const;

	// Services getters
	Application& currentApplication();
	Logger& logger();
	Profiler& profiler();

	// Settings
	void loadSettings();
	void saveSettings();

#if YAE_PLATFORM_WEB == 0
	// Rendering
	void initGl3w();
#endif

	// Modules
	const DataArray<Module*>& getModules() const;

// private:
	bool _doFrame();

	void _loadModule(Module* _module, const char* _dllPath);
	void _unloadModule(Module* _module);
	void _copyAndLoadModule(Module* _module);

	String _getModuleDLLPath(const char* _moduleName) const;
	String _getModuleSymbolsPath(const char* _moduleName) const;

	Logger* m_logger = nullptr;
	Profiler* m_profiler = nullptr;

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
	String m_settingsDirectory;
	String m_hotReloadDirectory;
	bool m_isInitialized = false;
	bool m_isGl3wInitialized = false;
	bool m_hotReloadEnabled = false;
	DataArray<Module*> m_modules;

	i32 m_previousConsoleWindowX = 0;
	i32 m_previousConsoleWindowY = 0;
	i32 m_previousConsoleWindowWidth = 0;
	i32 m_previousConsoleWindowHeight = 0;

	static Program* s_programInstance;
};

}
