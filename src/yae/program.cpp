#include "program.h"

#include <imgui/imgui.h>
#include <yae/platform.h>
#include <yae/filesystem.h>
#include <yae/resource.h>
#include <yae/application.h>
#include <yae/input.h>
#include <yae/profiler.h>
#include <yae/logger.h>
#include <yae/vulkan/VulkanRenderer.h>

// anonymous functions

void* ImGuiMemAlloc(size_t _size, void* _userData)
{
    return yae::toolAllocator().allocate(_size);
}

void ImGuiMemFree(void* _ptr, void* _userData)
{
    return yae::toolAllocator().deallocate(_ptr);
}

// !anonymous functions


namespace yae {

Program* Program::s_programInstance = nullptr;


Program::Program(Allocator* _defaultAllocator, Allocator* _scratchAllocator, Allocator* _toolAllocator)
	: m_defaultAllocator(_defaultAllocator)
	, m_scratchAllocator(_scratchAllocator)
	, m_toolAllocator(_toolAllocator)
	, m_exePath(_defaultAllocator)
	, m_binDirectory(_defaultAllocator)
	, m_rootDirectory(_defaultAllocator)
	, m_intermediateDirectory(_defaultAllocator)
	, m_hotReloadDirectory(_defaultAllocator)
	, m_applications(_defaultAllocator)
{

}


Program::~Program()
{
	m_toolAllocator = nullptr;
	m_scratchAllocator = nullptr;
	m_defaultAllocator = nullptr;
}


void Program::init(char** _args, int _argCount)
{
	YAE_ASSERT(s_programInstance == nullptr);
	YAE_ASSERT(_args != nullptr && _argCount >= 1);
	s_programInstance = this;	

	m_args = _args;
	m_argCount = _argCount;

	// @TODO: make an actual argument parser
	for (int i = 1; i < _argCount; ++i)
	{
		if (strcmp(_args[i], "-hotreload") == 0)
		{
			m_hotReloadGameAPI = true;
		}
	}

    m_logger = m_defaultAllocator->create<Logger>();

    m_exePath = filesystem::getAbsolutePath(m_args[0]);
    m_binDirectory = filesystem::getDirectory(m_exePath.c_str());
    m_rootDirectory = filesystem::getAbsolutePath((m_binDirectory + "/../../").c_str());
    m_intermediateDirectory = filesystem::getAbsolutePath((m_rootDirectory + "/intermediate/").c_str());
	m_hotReloadDirectory = m_intermediateDirectory + "dll/";

    // Setup directories
    filesystem::setWorkingDirectory(m_rootDirectory.c_str());
	filesystem::createDirectory(m_intermediateDirectory.c_str());
    filesystem::deletePath(m_hotReloadDirectory.c_str());
	filesystem::createDirectory(m_hotReloadDirectory.c_str());

	YAE_LOGF("exe path: %s",  getExePath());
	YAE_LOGF("bin directory: %s",  getBinDirectory());
	YAE_LOGF("root directory: %s",  getRootDirectory());
	YAE_LOGF("intermediate directory: %s",  getIntermediateDirectory());
	YAE_LOGF("working directory: %s", filesystem::getWorkingDirectory().c_str());

    ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);

	m_resourceManager = m_defaultAllocator->create<ResourceManager>();
	m_profiler = m_defaultAllocator->create<Profiler>(m_toolAllocator);

	// Prepare Game API for hot reload
	if (m_hotReloadGameAPI)
	{
		_copyAndLoadGameAPI(_getGameDLLPath().c_str(), _getGameDLLSymbolsPath().c_str());
	}
	else
	{
		_loadGameAPI(_getGameDLLPath().c_str());
	}

    /*
    logger.setCategoryVerbosity("glfw", yae::LogVerbosity_Verbose);
    logger.setCategoryVerbosity("vulkan", yae::LogVerbosity_Verbose);
    logger.setCategoryVerbosity("vulkan_internal", yae::LogVerbosity_Log);
    logger.setCategoryVerbosity("resource", yae::LogVerbosity_Verbose);
    */
}


void Program::shutdown()
{
	YAE_ASSERT(s_programInstance == this);

	m_resourceManager->flushResources();

	_unloadGameAPI();

	m_defaultAllocator->destroy(m_profiler);
	m_profiler = nullptr;

	m_defaultAllocator->destroy(m_resourceManager);
	m_resourceManager = nullptr;

	ImGui::SetAllocatorFunctions(nullptr, nullptr, nullptr);

	m_defaultAllocator->destroy(m_logger);
	m_logger = nullptr;

	m_defaultAllocator = nullptr;
	m_scratchAllocator = nullptr;
	m_toolAllocator = nullptr;

	s_programInstance = nullptr;
}


void Program::run()
{
	YAE_CAPTURE_START("init");
	for (Application* application : m_applications)
	{
		m_currentApplication = application;
		application->init(m_args, m_argCount);
	}
	m_currentApplication = nullptr;
    YAE_CAPTURE_STOP("init");

    {
        yae::String dump;
        m_profiler->dumpCapture("init", dump);
        printf(dump.c_str());
        printf("\n");
    }

    bool shouldExit = true;
    do
    {
		YAE_CAPTURE_START("frame");

    	shouldExit = true;
    	bool shouldReloadGameAPI = false;
    	for (Application* application : m_applications)
		{
			m_currentApplication = application;

			if (application->doFrame())
				shouldExit = false;

			// Force DLL reload
			if (application->input().isCtrlDown() && application->input().wasKeyJustPressed(GLFW_KEY_R))
			{
				shouldReloadGameAPI = true;
			}
		}

		// Hot Reload Game API
		if (m_hotReloadGameAPI)
		{
			// @TODO: Maybe do a proper file watch at some point. Adding a bit of wait seems to do the trick though
			u64 lastWriteTime = platform::getFileLastWriteTime(_getGameDLLPath().c_str());
			u64 timeSinceLastWriteTime = platform::getSystemTime() - lastWriteTime;
			bool isDLLOutDated = (lastWriteTime > m_gameDLLLastWriteTime && timeSinceLastWriteTime > 1000000); // 0.1 s
			shouldReloadGameAPI = shouldReloadGameAPI || isDLLOutDated;
			
			if (shouldReloadGameAPI)
			{
				_unloadGameAPI();
				_copyAndLoadGameAPI(_getGameDLLPath().c_str(), _getGameDLLSymbolsPath().c_str());
			}	
		}

#if YAE_PROFILING_ENABLED
		m_profiler->update();
#endif

    	YAE_CAPTURE_STOP("frame");
    }
    while (!shouldExit);
    m_currentApplication = nullptr;

    {
        yae::String dump;
        m_profiler->dumpCapture("frame", dump);
        printf(dump.c_str());
        printf("\n");
    }

    YAE_CAPTURE_START("shutdown");
    for (Application* application : m_applications)
	{
		m_currentApplication = application;
		application->renderer().waitIdle();

		application->shutdown();
	}
	m_currentApplication = nullptr;
    YAE_CAPTURE_STOP("shutdown");

    {
        yae::String dump;
        m_profiler->dumpCapture("shutdown", dump);
        printf(dump.c_str());
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


Application& Program::currentApplication()
{
	YAE_ASSERT(m_currentApplication != nullptr);
	return *m_currentApplication;
}


Allocator& Program::defaultAllocator()
{
	YAE_ASSERT(m_defaultAllocator != nullptr);
	return *m_defaultAllocator;
}


Allocator& Program::scratchAllocator()
{
	YAE_ASSERT(m_scratchAllocator != nullptr);
	return *m_scratchAllocator;
}


Allocator& Program::toolAllocator()
{
	YAE_ASSERT(m_toolAllocator != nullptr);
	return *m_toolAllocator;
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


void Program::initGame()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_gameAPI.libraryHandle != nullptr);

	m_gameAPI.gameInit();
}


void Program::updateGame()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_gameAPI.libraryHandle != nullptr);

	m_gameAPI.gameUpdate();
}


void Program::shutdownGame()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_gameAPI.libraryHandle != nullptr);

	m_gameAPI.gameShutdown();
}


void Program::_loadGameAPI(const char* _path)
{
	YAE_CAPTURE_FUNCTION();

	m_gameAPI.libraryHandle = platform::loadDynamicLibrary(_path);
	YAE_ASSERT(m_gameAPI.libraryHandle);

	m_gameAPI.gameInit = (GameFunctionPtr)platform::getProcedureAddress(m_gameAPI.libraryHandle, "initGame");
	m_gameAPI.gameUpdate = (GameFunctionPtr)platform::getProcedureAddress(m_gameAPI.libraryHandle, "updateGame");
	m_gameAPI.gameShutdown = (GameFunctionPtr)platform::getProcedureAddress(m_gameAPI.libraryHandle, "shutdownGame");
	m_gameAPI.onLibraryLoaded = (GameFunctionPtr)platform::getProcedureAddress(m_gameAPI.libraryHandle, "onLibraryLoaded");
	m_gameAPI.onLibraryUnloaded = (GameFunctionPtr)platform::getProcedureAddress(m_gameAPI.libraryHandle, "onLibraryUnloaded");

	YAE_ASSERT(m_gameAPI.gameInit);
	YAE_ASSERT(m_gameAPI.gameUpdate);
	YAE_ASSERT(m_gameAPI.gameShutdown);

	m_gameAPI.onLibraryLoaded();

	YAE_LOGF_CAT("game_module", "Loaded Game API from \"%s\"", _path);
}


void Program::_unloadGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_gameAPI.libraryHandle != nullptr);
	
	m_gameAPI.onLibraryUnloaded();
	
	platform::unloadDynamicLibrary(m_gameAPI.libraryHandle);
	m_gameAPI = {};

	YAE_LOG_CAT("game_module", "Unloaded Game API");
}


void Program::_copyAndLoadGameAPI(const char* _dllPath, const char* _symbolsPath)
{
	String dllDst = String(m_hotReloadDirectory + "game_hotreload.dll", &scratchAllocator());
	String symbolsDst = String(m_hotReloadDirectory + "game_hotreload.pdb", &scratchAllocator());

	filesystem::copy(_dllPath, dllDst.c_str(), filesystem::CopyMode_OverwriteExisting);
	filesystem::copy(_symbolsPath, symbolsDst.c_str(), filesystem::CopyMode_OverwriteExisting);

	_loadGameAPI(dllDst.c_str());

	m_gameDLLLastWriteTime = platform::getFileLastWriteTime(_dllPath);
}


String Program::_getGameDLLPath() const
{
	return m_binDirectory + "game.dll";
}


String Program::_getGameDLLSymbolsPath() const
{
	return m_binDirectory + "game.pdb";
}

} // namespace yae