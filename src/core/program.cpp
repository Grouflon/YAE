#include "program.h"

#include <core/platform.h>
#include <core/filesystem.h>
#include <core/profiler.h>
#include <core/logger.h>
#include <core/string.h>
#include <core/StringHashRepository.h>
#include <core/Module.h>
#include <core/serialization/serialization.h>
#include <core/serialization/JsonSerializer.h>

#if YAE_PLATFORM_WEB
#include <emscripten.h>
#define YAE_MODULE_EXTENSION "wasm"
#else
#define YAE_MODULE_EXTENSION "dll"
#endif

#include <core/yae_sdl.h>

#include <cstring>



// anonymous functions


#if YAE_PLATFORM_WEB
void DoProgramFrame()
{
	yae::program()._doFrame();
}
#endif

// !anonymous functions


struct ProgramSettings
{
	i32 consoleWindowWidth = -1;
	i32 consoleWindowHeight = -1;
	i32 consoleWindowX = -1;
	i32 consoleWindowY = -1;	
};
MIRROR_CLASS(ProgramSettings)
(
	MIRROR_MEMBER(consoleWindowWidth);
	MIRROR_MEMBER(consoleWindowHeight);
	MIRROR_MEMBER(consoleWindowX);
	MIRROR_MEMBER(consoleWindowY);
);

namespace yae {
Program* Program::s_programInstance = nullptr;

Program::Program()
	//: m_applications(&defaultAllocator())
	: m_exePath(&defaultAllocator())
	, m_binDirectory(&defaultAllocator())
	, m_rootDirectory(&defaultAllocator())
	, m_intermediateDirectory(&defaultAllocator())
	, m_settingsDirectory(&defaultAllocator())
	, m_hotReloadDirectory(&defaultAllocator())
	, m_modules(&defaultAllocator())
{
	YAE_ASSERT(s_programInstance == nullptr);
	s_programInstance = this;
    m_logger = defaultAllocator().create<Logger>();
}


Program::~Program()
{
	YAE_ASSERT(s_programInstance == this);

	for (Module* module : m_modules)
	{
		defaultAllocator().destroy(module);
	}
	m_modules.clear();
	m_modulesByName.clear();
	
	defaultAllocator().destroy(m_logger);
	m_logger = nullptr;
	s_programInstance = nullptr;
}

Module* Program::registerModule(const char* _moduleName)
{
	YAE_ASSERT_MSG(!m_isInitialized, "Modules but be registered before the program is initialized");

	StringHash moduleNameHash = StringHash(_moduleName);
	YAE_ASSERT_MSGF(m_modulesByName.get(moduleNameHash) == nullptr, "Module \"%s\" has been registered twice.", _moduleName);

	Module* module = defaultAllocator().create<Module>();
	module->name = _moduleName;
	m_modulesByName.set(moduleNameHash, module);
	m_modules.push_back(module);
	return module;
}

Module* Program::findModule(const char* _moduleName) const
{
	StringHash moduleNameHash(_moduleName);
	return findModule(moduleNameHash);	
}

Module* Program::findModule(StringHash _moduleNameHash) const
{
	Module*const* modulePtr = m_modulesByName.get(_moduleNameHash);
	return modulePtr != nullptr ? *modulePtr : nullptr;
}

void Program::reloadModule(const char* _moduleName)
{
	Module* module = findModule(_moduleName);
	if (module == nullptr)
	{
		YAE_ERRORF("Can't find module \"%s\" to reload.", _moduleName);
	}
	reloadModule(module);
}

void Program::reloadModule(Module* _module)
{
	if (_module == nullptr)
		return;

	m_modulesToReload.push_back(_module);
}

void Program::init(const char** _args, int _argCount)
{
	YAE_ASSERT(s_programInstance == this);

	m_isInitialized = true;

	YAE_ASSERT(_args != nullptr && _argCount >= 1);

	m_args = _args;
	m_argCount = _argCount;

	// @TODO: make an actual argument parser
	for (int i = 1; i < _argCount; ++i)
	{
		if (strcmp(_args[i], "-hotreload") == 0)
		{
			m_hotReloadEnabled = true;
		}
	}

	m_profiler = defaultAllocator().create<Profiler>(&toolAllocator());

	YAE_CAPTURE_FUNCTION();

    m_exePath = filesystem::getAbsolutePath(m_args[0]);
    m_binDirectory = filesystem::getDirectory(m_exePath.c_str());
    m_rootDirectory = filesystem::getAbsolutePath((m_binDirectory + "/../../").c_str());
    m_intermediateDirectory = filesystem::getAbsolutePath((m_rootDirectory + "/intermediate/").c_str());
	m_hotReloadDirectory = m_intermediateDirectory + "dll/";
	m_settingsDirectory = m_intermediateDirectory + "settings/";

    // Setup directories
    filesystem::setWorkingDirectory(m_rootDirectory.c_str());

#if YAE_PLATFORM_WEB == 0
	filesystem::createDirectory(m_intermediateDirectory.c_str());
    filesystem::deletePath(m_hotReloadDirectory.c_str());
	filesystem::createDirectory(m_hotReloadDirectory.c_str());
	filesystem::createDirectory(m_settingsDirectory.c_str());
#endif

	YAE_LOGF("exe path: %s",  getExePath());
	YAE_LOGF("bin directory: %s",  getBinDirectory());
	YAE_LOGF("root directory: %s",  getRootDirectory());
	YAE_LOGF("intermediate directory: %s",  getIntermediateDirectory());
	YAE_LOGF("settings directory: %s",  getSettingsDirectory());
	YAE_LOGF("working directory: %s", filesystem::getWorkingDirectory().c_str());

	// SDL Init
	{
		u32 flags = SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER;
		YAE_SDL_VERIFY(SDL_Init(flags));
	}

	for (Module* module : m_modules)
	{	
		// Prepare Game API for hot reload
		if (m_hotReloadEnabled)
		{
			_copyAndLoadModule(module);
		}
		else
		{
			_loadModule(module, _getModuleDLLPath(module->name.c_str()).c_str());
		}
	}

	if (void* consoleWindowHandle = platform::findConsoleWindowHandle())
	{
		platform::getWindowSize(consoleWindowHandle, &m_previousConsoleWindowWidth, &m_previousConsoleWindowHeight);
		platform::getWindowPosition(consoleWindowHandle, &m_previousConsoleWindowX, &m_previousConsoleWindowY);
	}
	loadSettings();
}


void Program::shutdown()
{
	YAE_ASSERT(s_programInstance == this);

	for (int i = m_modules.size() - 1; i >= 0; --i)
	{
		// @NOTE(remi): unload modules in reverse order to preserve symetry
		_unloadModule(m_modules[i]);
	}

	// SDL shutdown
	{
		SDL_Quit();
	}

	defaultAllocator().destroy(m_profiler);
	m_profiler = nullptr;

#if DEBUG_STRINGHASH
	clearStringHashRepository();
#endif

	m_isInitialized = false;
}

void Program::run()
{
	YAE_CAPTURE_START("init_modules");
	for (Module* module : m_modules)
	{
		if (module->initModuleFunction != nullptr)
		{
			module->initModuleFunction(this, module);	
		}
	}

	for (Module* module : m_modules)
	{
		if (module->startProgramFunction != nullptr)
		{
			module->startProgramFunction(this, module);
		}
	}

    YAE_CAPTURE_STOP("init_modules");
    {
        yae::String dump;
        m_profiler->dumpCapture("init_modules", dump);
        printf("%s", dump.c_str());
        printf("\n");
    }

#if YAE_PLATFORM_WEB
    emscripten_set_main_loop(&DoProgramFrame, 0, true);
#else
    do
    {
    	_doFrame();
    }
    while (!m_exitRequested);
#endif

    {
        yae::String dump;
        m_profiler->dumpCapture("frame", dump);
        printf("%s", dump.c_str());
        printf("\n");
    }

    YAE_CAPTURE_START("shutdown");

    for (Module* module : m_modules)
	{
		if (module->stopProgramFunction != nullptr)
		{
			module->stopProgramFunction(this, module);	
		}
	}

	for (int i = m_modules.size() - 1; i >= 0; --i)
	{
		// @NOTE(remi): shutdown modules in reverse order to preserve symetry
		Module* module = m_modules[i];
		if (module->shutdownModuleFunction != nullptr)
		{
			module->shutdownModuleFunction(this, module);	
		}
	}
    YAE_CAPTURE_STOP("shutdown");

    {
        yae::String dump;
        m_profiler->dumpCapture("shutdown", dump);
        printf("%s", dump.c_str());
        printf("\n");
    }
}

void Program::requestExit()
{
	m_exitRequested = true;
}

const char** Program::args() const
{
	return m_args;
}

int Program::argCount() const
{
	return m_argCount;
}

const char* Program::getExePath() const
{
	return m_exePath.c_str();
}

const char* Program::getBinDirectory() const
{
	return m_binDirectory.c_str();
}

const char* Program::getRootDirectory() const
{
	return m_rootDirectory.c_str();
}


const char* Program::getIntermediateDirectory() const
{
	return m_intermediateDirectory.c_str();
}

const char* Program::getSettingsDirectory() const
{
	return m_settingsDirectory.c_str();
}

Logger& Program::logger()
{
	YAE_ASSERT(m_logger != nullptr);
	return *m_logger;
}

Profiler& Program::profiler()
{
	YAE_ASSERT(m_profiler != nullptr);
	return *m_profiler;
}

static String getSettingsFilePath()
{
	return filesystem::normalizePath(string::format("%s/program_settings.json", program().getSettingsDirectory()).c_str());
}

static bool serializeSettings(Serializer* _serializer, ProgramSettings* _settings)
{
	return serialization::serializeMirrorType(_serializer, *_settings);
}

void Program::loadSettings()
{
	String filePath = getSettingsFilePath();
	FileReader reader(filePath.c_str());
	if (!reader.load())
	{
		// No settings file, do nothing
		return;
	}

	JsonSerializer serializer(&scratchAllocator());
	if (!serializer.parseSourceData(reader.getContent(), reader.getContentSize()))
	{
		YAE_ERRORF_CAT("program", "Failed to parse json settings file \"%s\"", filePath.c_str());
		return;	
	}

	ProgramSettings settings;
	serializer.beginRead();
	serializeSettings(&serializer, &settings);
	serializer.endRead();

	if (void* consoleWindowHandle = platform::findConsoleWindowHandle())
	{
		if (settings.consoleWindowWidth > 0 && settings.consoleWindowHeight > 0)
		{
			platform::setWindowSize(consoleWindowHandle, settings.consoleWindowWidth, settings.consoleWindowHeight);
		}
		if (settings.consoleWindowX != -1 && settings.consoleWindowY != -1)
		{
			platform::setWindowPosition(consoleWindowHandle, settings.consoleWindowX, settings.consoleWindowY);
		}
		m_previousConsoleWindowX = settings.consoleWindowX;
		m_previousConsoleWindowY = settings.consoleWindowY;
		m_previousConsoleWindowWidth = settings.consoleWindowWidth;
		m_previousConsoleWindowHeight = settings.consoleWindowHeight;
	}
	YAE_LOGF_CAT("program", "Loaded program settings from \"%s\"", filePath.c_str());
}

void Program::saveSettings()
{
	ProgramSettings settings;
	if (void* consoleWindowHandle = platform::findConsoleWindowHandle())
	{
		platform::getWindowSize(consoleWindowHandle, &settings.consoleWindowWidth, &settings.consoleWindowHeight);
		platform::getWindowPosition(consoleWindowHandle, &settings.consoleWindowX, &settings.consoleWindowY);
	}

	JsonSerializer serializer(&scratchAllocator());
	serializer.beginWrite();
	serializeSettings(&serializer, &settings);
	serializer.endWrite();

	String filePath = getSettingsFilePath();
	FileHandle file(filePath.c_str());
	if (!file.open(FileHandle::OPENMODE_WRITE))
	{
		YAE_ERRORF_CAT("program", "Failed to open \"%s\" for write", filePath.c_str());
		return;
	}
	if (!file.write(serializer.getWriteData(), serializer.getWriteDataSize()))
	{
		YAE_ERRORF_CAT("program", "Failed to write into \"%s\"", filePath.c_str());
		return;
	}
	file.close();

	YAE_LOGF_CAT("program", "Saved program settings to \"%s\"", filePath.c_str());
}

void Program::registerFunctionPointer(void** _pointerLocation)
{
	YAE_ASSERT(!m_functionPointersPatch.has(_pointerLocation));
	m_functionPointersPatch.set(_pointerLocation, FunctionPointerPatch());
}

void Program::unregisterFunctionPointer(void** _pointerLocation)
{
	YAE_ASSERT(m_functionPointersPatch.has(_pointerLocation));
	m_functionPointersPatch.remove(_pointerLocation);
}

const DataArray<Module*>& Program::getModules() const
{
	return m_modules;	
}

void Program::_doFrame()
{
	// the profiler::cleanEvents function is not all right and remove events while their indices are still in the stack
	// let's come back to it later
	//YAE_CAPTURE_FUNCTION();

	YAE_CAPTURE_START("frame");

	for (Module* module : m_modules)
	{
		if (module->updateProgramFunction != nullptr)
		{
			module->updateProgramFunction(this, module);	
		}
	}

	// Hot Reload Modules
	if (m_hotReloadEnabled)
	{
		_processModuleHotReload();
	}

	// Settings save check
	if (void* consoleWindowHandle = platform::findConsoleWindowHandle())
	{
		i32 x, y, w, h;
		platform::getWindowSize(consoleWindowHandle, &w, &h);
		platform::getWindowPosition(consoleWindowHandle, &x, &y);
		if (
			m_previousConsoleWindowX != x || m_previousConsoleWindowY != y ||
			m_previousConsoleWindowWidth != w || m_previousConsoleWindowHeight != h
		)
		{
			saveSettings();
		}
		m_previousConsoleWindowX = x;
		m_previousConsoleWindowY = y;
		m_previousConsoleWindowWidth = w;
		m_previousConsoleWindowHeight = h;
	}

	/*
	if (!shouldContinue)
	{
		m_profiler->dropCapture("frame");
	}
	else
	{
		YAE_CAPTURE_STOP("frame");
	}*/
	YAE_CAPTURE_STOP("frame");

#if YAE_PROFILING_ENABLED
	m_profiler->update();
#endif
}


void Program::_loadModule(Module* _module, const char* _dllPath)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_module->libraryHandle == nullptr);

	_module->libraryHandle = platform::loadDynamicLibrary(_dllPath);
	YAE_ASSERT(_module->libraryHandle);

	mirror::InitNewTypes();

	_module->beforeModuleReloadFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "beforeModuleReload");
	_module->afterModuleReloadFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "afterModuleReload");
	_module->initModuleFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "initModule");
	_module->shutdownModuleFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "shutdownModule");
	_module->startProgramFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "startProgram");
	_module->stopProgramFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "stopProgram");
	_module->updateProgramFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "updateProgram");
	_module->getDependenciesFunction = (void (*)(const char***, int*))platform::getProcedureAddress(_module->libraryHandle, "getDependencies");

	if (_module->getDependenciesFunction != nullptr)
	{
		const char** moduleNames;
		int moduleCount;
		_module->getDependenciesFunction(&moduleNames, &moduleCount);
		for (int i = 0; i < moduleCount; ++i)
		{
			StringHash moduleNameHash = StringHash(moduleNames[i]);
			YAE_ASSERT(_module->dependencies.find(moduleNameHash) == nullptr);
			_module->dependencies.push_back(moduleNameHash);
			Module* dependency = program().findModule(moduleNameHash);
			YAE_ASSERT(dependency != nullptr);
			dependency->dependents.push_back(StringHash(_module->name.c_str()));
		}
	}

	YAE_LOGF_CAT("program", "Loaded \"%s\" module from \"%s\"", _module->name.c_str(), _dllPath);
}


void Program::_unloadModule(Module* _module)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_module->libraryHandle != nullptr);

	StringHash unloadedModuleNameHash = StringHash(_module->name.c_str());
	for (StringHash moduleNameHash : _module->dependencies)
	{
		Module* dependency = program().findModule(moduleNameHash);
		YAE_ASSERT(dependency != nullptr);
		auto it = dependency->dependents.find(unloadedModuleNameHash);
		YAE_ASSERT(it != nullptr);
		dependency->dependents.erase(it);
	}
	_module->dependencies.clear();
	
	_module->beforeModuleReloadFunction = nullptr;
	_module->afterModuleReloadFunction = nullptr;
	_module->initModuleFunction = nullptr;
	_module->shutdownModuleFunction = nullptr;
	_module->startProgramFunction = nullptr;
	_module->stopProgramFunction = nullptr;
	_module->updateProgramFunction = nullptr;
	_module->getDependenciesFunction = nullptr;

	platform::unloadDynamicLibrary(_module->libraryHandle);
	_module->libraryHandle = nullptr;

	YAE_LOGF_CAT("program", "Unloaded \"%s\" module", _module->name.c_str());
}


void Program::_copyAndLoadModule(Module* _module)
{
	String dllSrc = _getModuleDLLPath(_module->name.c_str());
	String SymbolsSrc = _getModuleSymbolsPath(_module->name.c_str());

	String dllDst = string::format("%s%s.%s", m_hotReloadDirectory.c_str(), _module->name.c_str(), YAE_MODULE_EXTENSION);
	String symbolsDst = string::format("%s%s.pdb", m_hotReloadDirectory.c_str(), _module->name.c_str());

	YAE_VERIFY(filesystem::copy(dllSrc.c_str(), dllDst.c_str(), filesystem::CopyMode_OverwriteExisting));
	YAE_VERIFY(filesystem::copy(SymbolsSrc.c_str(), symbolsDst.c_str(), filesystem::CopyMode_OverwriteExisting));

	_loadModule(_module, dllDst.c_str());

	_module->lastLibraryWriteTime = filesystem::getFileLastWriteTime(dllSrc.c_str());
}

void Program::_processModuleHotReload()
{
	for (Module* module : m_modules)
	{
		// @TODO: Maybe do a proper file watch at some point. Adding a bit of wait seems to do the trick though
		String dllPath = _getModuleDLLPath(module->name.c_str());
		Date lastWriteTime = filesystem::getFileLastWriteTime(dllPath.c_str());
		Date timeSinceLastWriteTime = date::now() - lastWriteTime;
		bool isDLLOutDated = (lastWriteTime > module->lastLibraryWriteTime && timeSinceLastWriteTime > 0);
		if (isDLLOutDated)
		{
			m_modulesToReload.push_back(module);
		}
	}

	if (m_modulesToReload.size() > 0)
	{
		YAE_CAPTURE_SCOPE("ReloadGameAPI");

		// NOTE: Commenting out until patch is actually working
		//_prepareFunctionPointersPatch();

		// NOTE: let's do something dumb for now: unload everything, reload everything

		for (int i = m_modules.size() - 1; i >= 0; --i)
		{
			Module* module = m_modules[i];

			if (module->beforeModuleReloadFunction != nullptr)
			{
				module->beforeModuleReloadFunction(this, module);
			}

			_unloadModule(module);
		}

		for (Module* module : m_modules)
		{
			_copyAndLoadModule(module);

			if (module->afterModuleReloadFunction != nullptr)
			{
				module->afterModuleReloadFunction(this, module);
			}
		}

		// NOTE: Commenting out until patch is actually working
		//_patchFunctionPointers();

		/*
		for (int i = modulesToReload.size() - 1; i >= 0; --i)
		{
			Module* module = modulesToReload[i];
			_unloadModule(module);
		}

		for (Module* module : modulesToReload)
		{
			_copyAndLoadModule(module);

			if (module->onModuleReloadedFunction != nullptr)
			{
				module->onModuleReloadedFunction(this, module);
			}
		}
		*/
	}
	m_modulesToReload.clear();
}

String Program::_getModuleDLLPath(const char* _moduleName) const
{
	return string::format("%s%s.%s", m_binDirectory.c_str(), _moduleName, YAE_MODULE_EXTENSION);
}


String Program::_getModuleSymbolsPath(const char* _moduleName) const
{
	return string::format("%s%s.pdb", m_binDirectory.c_str(), _moduleName);
}

void Program::_prepareFunctionPointersPatch()
{
	platform::loadSymbols();
	for (auto& pair : m_functionPointersPatch)
	{
		pair.value.address = *pair.key;
		if (pair.value.address == nullptr)
			continue;

		pair.value.name = platform::getSymbolNameFromAddress(pair.value.address);
		YAE_VERBOSEF_CAT("function_patch", "Prepare %s(%p) patch", pair.value.name.c_str(), pair.value.address);
	}
	platform::unloadSymbols();
}

void Program::_patchFunctionPointers()
{
	platform::loadSymbols();
	for (auto& pair : m_functionPointersPatch)
	{
		if (pair.value.address == nullptr)
			continue;

		void* newAddress = platform::getAddressFromSymbolName(pair.value.name.c_str());
		YAE_ASSERT(newAddress != nullptr);
		YAE_VERBOSEF_CAT("function_patch", "Patching %s(%p->%p) patch", pair.value.name.c_str(), pair.value.address, newAddress);

		if (newAddress != pair.value.address)
		{
			*pair.key = newAddress;
			logger().setDefaultOutputColor(OutputColor_Green);
			YAE_LOGF("Patch %s successful: %p -> %p", pair.value.name.c_str(), pair.value.address, newAddress);
			logger().setDefaultOutputColor(OutputColor_Default);

		}
	}
	platform::unloadSymbols();
}

} // namespace yae