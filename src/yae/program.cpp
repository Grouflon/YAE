#include "program.h"

#include <imgui/imgui.h>
#include <yae/log.h>
#include <yae/platform.h>
#include <yae/game_module.h>
#include <yae/resource.h>
#include <yae/profiling.h>
#include <yae/application.h>
#include <yae/input.h>
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

    m_logger = m_defaultAllocator->create<Logger>();

    m_exePath = Path(m_args[0], m_scratchAllocator);
	YAE_LOG(m_exePath.c_str());
	String workingDirectory = platform::getWorkingDirectory();
	Path workingDirectoryPath(workingDirectory.c_str());
	YAE_LOG(workingDirectoryPath.c_str());

    ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);

	m_resourceManager = m_defaultAllocator->create<ResourceManager>();
	m_profiler = m_defaultAllocator->create<Profiler>(m_toolAllocator);

    yae::loadGameAPI();

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

    yae::unloadGameAPI();

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

		// Game API
		if (shouldReloadGameAPI)
		{
			unloadGameAPI();
			loadGameAPI();
		}
		watchGameAPI();

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


const Path& Program::getExePath() const
{
	return m_exePath;
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


} // namespace yae