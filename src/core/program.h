#pragma once

#include <core/types.h>
#include <core/Date.h>
#include <core/StringHash.h>
#include <core/containers/Array.h>
#include <core/containers/HashMap.h>

namespace lpp
{
	struct LppSynchronizedAgent;
}

namespace yae {

class Allocator;
class Logger;
class Profiler;
class Module;

// @TODO: Rename as Core
class CORE_API Program
{
public:
	Program();
	~Program();

	Module* registerModule(const char* _moduleName);
	Module* findModule(const char* _moduleName) const;
	Module* findModule(StringHash _moduleNameHash) const;

	bool isCodeHotReloadEnabled() const;
	void requestCodeHotReload();
	void requestHotRestart();

	void init(const char** _args, int _argCount);
	void shutdown();

	void run();
	void requestExit();

	// Arguments
	const char** args() const;
	int argCount() const;

	// Paths & Directories
	const char* getExePath() const;
	const char* getBinDirectory() const;
	const char* getRootDirectory() const;
	const char* getIntermediateDirectory() const;
	const char* getSettingsDirectory() const;

	// Services getters
	Logger& logger();
	Profiler& profiler();

	// Settings
	void loadSettings();
	void saveSettings();

	// Modules
	const DataArray<Module*>& getModules() const;

// private:
	void _doFrame();

	void _loadModule(Module* _module, const char* _dllPath);
	void _unloadModule(Module* _module);
	void _copyAndLoadModule(Module* _module);
	void _processModuleHotReload();

	String _getModuleDLLPath(const char* _moduleName) const;
	String _getModuleSymbolsPath(const char* _moduleName) const;

	void _onSerialize(Serializer& _serializer);

	Logger* m_logger = nullptr;
	Profiler* m_profiler = nullptr;
	lpp::LppSynchronizedAgent* m_lppAgent;

	int m_argCount = 0;
	const char** m_args = nullptr;
	String m_exePath;
	String m_binDirectory;
	String m_rootDirectory;
	String m_intermediateDirectory;
	String m_settingsDirectory;
	String m_hotReloadDirectory;
	bool m_isInitialized = false;
	bool m_hotReloadEnabled = false;
	DataArray<Module*> m_modules;
	HashMap<StringHash, Module*> m_modulesByName;
	bool m_exitRequested = false;

	i32 m_previousConsoleWindowX = 0;
	i32 m_previousConsoleWindowY = 0;
	i32 m_previousConsoleWindowWidth = 0;
	i32 m_previousConsoleWindowHeight = 0;

	static Program* s_programInstance;
};

}
