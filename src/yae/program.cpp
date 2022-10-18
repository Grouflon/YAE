#include "program.h"

#include <yae/platform.h>
#include <yae/filesystem.h>
#include <yae/resource.h>
#include <yae/Application.h>
#include <yae/profiler.h>
#include <yae/logger.h>
#include <yae/hash.h>
#include <yae/Module.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/JsonSerializer.h>
#include <yae/resources/FileResource.h>

#if YAE_PLATFORM_WEB
#include <emscripten.h>
#define YAE_MODULE_EXTENSION "wasm"
#else
#include <GL/gl3w.h>
#define YAE_MODULE_EXTENSION "dll"
#endif

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>


// anonymous functions

void GLFWErrorCallback(int _error, const char* _description)
{
    YAE_ERRORF_CAT("glfw", "Glfw Error 0x%04x: %s", _error, _description);
}

void* ImGuiMemAlloc(size_t _size, void* _userData)
{
    return yae::toolAllocator().allocate(_size);
}

void ImGuiMemFree(void* _ptr, void* _userData)
{
    return yae::toolAllocator().deallocate(_ptr);
}

#if YAE_PLATFORM_WEB
void DoProgramFrame()
{
	yae::program()._doFrame();
}
#endif

// !anonymous functions


namespace yae {

struct ProgramSettings
{
	i32 consoleWindowWidth = -1;
	i32 consoleWindowHeight = -1;
	i32 consoleWindowX = -1;
	i32 consoleWindowY = -1;

	MIRROR_CLASS_NOVIRTUAL(ProgramSettings)
	(
		MIRROR_MEMBER(consoleWindowWidth)();
		MIRROR_MEMBER(consoleWindowHeight)();
		MIRROR_MEMBER(consoleWindowX)();
		MIRROR_MEMBER(consoleWindowY)();
	);
};
MIRROR_CLASS_DEFINITION(ProgramSettings);

Program* Program::s_programInstance = nullptr;

Program::Program()
	: m_applications(&defaultAllocator())
	, m_exePath(&defaultAllocator())
	, m_binDirectory(&defaultAllocator())
	, m_rootDirectory(&defaultAllocator())
	, m_intermediateDirectory(&defaultAllocator())
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
	
	defaultAllocator().destroy(m_logger);
	m_logger = nullptr;
	s_programInstance = nullptr;
}

void Program::registerModule(const char* _moduleName)
{
	YAE_ASSERT_MSG(!m_isInitialized, "Modules but be registered before the program is initialized");

#if YAE_ASSERT_ENABLED
	for (const Module* module : m_modules)
	{
		YAE_ASSERT_MSGF(module->name != _moduleName, "Module \"%s\" has been registered twice.", _moduleName);
	}
#endif
	Module* module = defaultAllocator().create<Module>();
	module->name = _moduleName;
	m_modules.push_back(module);
}

void Program::init(char** _args, int _argCount)
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

	// Init GLFW
	{
		YAE_CAPTURE_SCOPE("glfwInit");
	    glfwSetErrorCallback(&GLFWErrorCallback);

		int result = glfwInit();
		YAE_ASSERT(result == GLFW_TRUE);
		YAE_VERBOSE_CAT("glfw", "Initialized glfw");
	}

	// Init ImGui
	{
    	ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);
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

	m_resourceManager = defaultAllocator().create<ResourceManager>();

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

	m_resourceManager->flushResources();
	defaultAllocator().destroy(m_resourceManager);
	m_resourceManager = nullptr;

	for (int i = m_modules.size() - 1; i >= 0; --i)
	{
		// @NOTE(remi): unload modules in reverse order to preserve symetry
		_unloadModule(m_modules[i]);
	}

	// ImGui shutdown
	{
		ImGui::SetAllocatorFunctions(nullptr, nullptr, nullptr);
	}

	// glfw shutdown
	{
		YAE_CAPTURE_SCOPE("glfwTerminate");

		glfwTerminate();
		YAE_VERBOSE_CAT("glfw", "Terminated glfw");	
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
	YAE_CAPTURE_START("init");
	for (Module* module : m_modules)
	{
		if (module->initModuleFunction != nullptr)
		{
			module->initModuleFunction(this, module);	
		}
	}

	for (Application* application : m_applications)
	{
		m_currentApplication = application;

		for (Module* module : m_modules)
		{
			if (module->beforeInitApplicationFunction != nullptr)
			{
				module->beforeInitApplicationFunction(application);
			}
		}

		application->init(m_args, m_argCount);

		for (Module* module : m_modules)
		{
			if (module->afterInitApplicationFunction != nullptr)
			{
				module->afterInitApplicationFunction(application);
			}
		}
	}
	m_currentApplication = nullptr;
    YAE_CAPTURE_STOP("init");

    {
        yae::String dump;
        m_profiler->dumpCapture("init", dump);
        printf("%s", dump.c_str());
        printf("\n");
    }

#if YAE_PLATFORM_WEB
    emscripten_set_main_loop(&DoProgramFrame, 0, true);
#else
    bool shouldContinue = false;
    do
    {
    	shouldContinue = _doFrame();
    }
    while (shouldContinue);
#endif

    {
        yae::String dump;
        m_profiler->dumpCapture("frame", dump);
        printf("%s", dump.c_str());
        printf("\n");
    }

    YAE_CAPTURE_START("shutdown");
    for (Application* application : m_applications)
	{
		m_currentApplication = application;

		for (int i = m_modules.size() - 1; i >= 0; --i)
		{
			// @NOTE(remi): shutdown modules in reverse order to preserve symetry
			Module* module = m_modules[i];
			if (module->beforeShutdownApplicationFunction != nullptr)
			{
				module->beforeShutdownApplicationFunction(application);
			}
		}

		application->shutdown();

		for (int i = m_modules.size() - 1; i >= 0; --i)
		{
			// @NOTE(remi): shutdown modules in reverse order to preserve symetry
			Module* module = m_modules[i];
			if (module->afterShutdownApplicationFunction != nullptr)
			{
				module->afterShutdownApplicationFunction(application);
			}
		}
	}
	m_currentApplication = nullptr;

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

void Program::registerApplication(Application* _application)
{
	YAE_ASSERT(_application != nullptr);
	YAE_ASSERT(m_applications.find(_application) == nullptr);
	m_applications.push_back(_application);
}

void Program::unregisterApplication(Application* _application)
{
	YAE_ASSERT(_application != nullptr);
	auto match = m_applications.find(_application);
	YAE_ASSERT(match != nullptr);
	m_applications.erase(match);
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

Application& Program::currentApplication()
{
	YAE_ASSERT(m_currentApplication != nullptr);
	return *m_currentApplication;
}

ResourceManager& Program::resourceManager()
{
	YAE_ASSERT(m_resourceManager != nullptr);
	return *m_resourceManager;
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
	FileResource* settingsFile = findOrCreateResource<FileResource>(filePath.c_str());
	if (!settingsFile->useLoad())
	{
		YAE_ERRORF_CAT("program", "Failed to load settings file \"%s\"", filePath.c_str());
		settingsFile->releaseUnuse();
		return;
	}

	JsonSerializer serializer(&scratchAllocator());
	if (!serializer.parseSourceData(settingsFile->getContent(), settingsFile->getContentSize()))
	{
		YAE_ERRORF_CAT("program", "Failed to parse json settings file \"%s\"", filePath.c_str());
		settingsFile->releaseUnuse();
		return;	
	}
	settingsFile->releaseUnuse();

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

#if YAE_PLATFORM_WEB == 0
void Program::initGl3w()
{
	// @NOTE(remi): this needs to be done once a window is created so it has to be called from the yae module
	if (!m_isGl3wInitialized)
	{
		YAE_CAPTURE_SCOPE("gl3wInit");
		int result = gl3wInit();
		YAE_ASSERT(result == GL3W_OK);
		m_isGl3wInitialized = true;
	}
}
#endif

const DataArray<Module*>& Program::getModules() const
{
	return m_modules;	
}

bool Program::_doFrame()
{
	// the profiler::cleanEvents function is not all right and remove events while their indices are still in the stack
	// let's come back to it later
	//YAE_CAPTURE_FUNCTION();

	YAE_CAPTURE_START("frame");

	bool shouldContinue = false;
	for (Application* application : m_applications)
	{
		m_currentApplication = application;

		if (application->doFrame())
			shouldContinue = true;
	}
	m_currentApplication = nullptr;

	// Hot Reload Game API
	if (m_hotReloadEnabled)
	{

		DataArray<Module*> modulesToReload(&scratchAllocator());
		for (Module* module : m_modules)
		{
			// @TODO: Maybe do a proper file watch at some point. Adding a bit of wait seems to do the trick though
			String dllPath = _getModuleDLLPath(module->name.c_str());
			Date lastWriteTime = filesystem::getFileLastWriteTime(dllPath.c_str());
			Date timeSinceLastWriteTime = date::now() - lastWriteTime;
			bool isDLLOutDated = (lastWriteTime > module->lastLibraryWriteTime && timeSinceLastWriteTime > 0);
			if (isDLLOutDated)
			{
				modulesToReload.push_back(module);
			}
		}

		if (modulesToReload.size() > 0)
		{
			YAE_CAPTURE_SCOPE("ReloadGameAPI");

			for (int i = modulesToReload.size() - 1; i >= 0; --i)
			{
				Module* module = modulesToReload[i];
				_unloadModule(module);
			}

			for (Module* module : modulesToReload)
			{
				_copyAndLoadModule(module);
			}
		}
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

	if (!shouldContinue)
	{
		m_profiler->dropCapture("frame");
	}
	else
	{
		YAE_CAPTURE_STOP("frame");
	}


#if YAE_PROFILING_ENABLED
	m_profiler->update();
#endif

	return shouldContinue;
}


void Program::_loadModule(Module* _module, const char* _dllPath)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_module->libraryHandle == nullptr);

	_module->libraryHandle = platform::loadDynamicLibrary(_dllPath);
	YAE_ASSERT(_module->libraryHandle);

	mirror::GetTypeSet().resolveTypes();

	_module->onModuleLoadedFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "onModuleLoaded");
	_module->onModuleUnloadedFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "onModuleUnloaded");
	_module->initModuleFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "initModule");
	_module->shutdownModuleFunction = (void (*)(Program*, Module*))platform::getProcedureAddress(_module->libraryHandle, "shutdownModule");
	_module->beforeInitApplicationFunction = (void (*)(Application*))platform::getProcedureAddress(_module->libraryHandle, "beforeInitApplication");
	_module->afterInitApplicationFunction = (void (*)(Application*))platform::getProcedureAddress(_module->libraryHandle, "afterInitApplication");
	_module->updateApplicationFunction = (void (*)(Application*, float))platform::getProcedureAddress(_module->libraryHandle, "updateApplication");
	_module->beforeShutdownApplicationFunction = (void (*)(Application*))platform::getProcedureAddress(_module->libraryHandle, "beforeShutdownApplication");
	_module->afterShutdownApplicationFunction = (void (*)(Application*))platform::getProcedureAddress(_module->libraryHandle, "afterShutdownApplication");
	_module->onSerializeApplicationSettingsFunction = (bool (*)(Application*, Serializer*))platform::getProcedureAddress(_module->libraryHandle, "onSerializeApplicationSettings");

	if (_module->onModuleLoadedFunction != nullptr)
	{
		_module->onModuleLoadedFunction(this, _module);
	}

	YAE_LOGF_CAT("program", "Loaded \"%s\" module from \"%s\"", _module->name.c_str(), _dllPath);
}


void Program::_unloadModule(Module* _module)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_module->libraryHandle != nullptr);
	
	if (_module->onModuleUnloadedFunction != nullptr)
	{
		_module->onModuleUnloadedFunction(this, _module);
	}
	
	_module->onModuleLoadedFunction = nullptr;
	_module->onModuleUnloadedFunction = nullptr;
	_module->initModuleFunction = nullptr;
	_module->shutdownModuleFunction = nullptr;
	_module->beforeInitApplicationFunction = nullptr;
	_module->afterInitApplicationFunction = nullptr;
	_module->updateApplicationFunction = nullptr;
	_module->beforeShutdownApplicationFunction = nullptr;
	_module->afterShutdownApplicationFunction = nullptr;
	_module->onSerializeApplicationSettingsFunction = nullptr;

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

	filesystem::copy(dllSrc.c_str(), dllDst.c_str(), filesystem::CopyMode_OverwriteExisting);
	filesystem::copy(SymbolsSrc.c_str(), symbolsDst.c_str(), filesystem::CopyMode_OverwriteExisting);

	_loadModule(_module, dllDst.c_str());

	//filesystem::deletePath(dllSrc.c_str());
	//filesystem::deletePath(SymbolsSrc.c_str());

	_module->lastLibraryWriteTime = filesystem::getFileLastWriteTime(dllSrc.c_str());
}


String Program::_getModuleDLLPath(const char* _moduleName) const
{
	return string::format("%s%s.%s", m_binDirectory.c_str(), _moduleName, YAE_MODULE_EXTENSION);
}


String Program::_getModuleSymbolsPath(const char* _moduleName) const
{
	return string::format("%s%s.pdb", m_binDirectory.c_str(), _moduleName);
}

} // namespace yae