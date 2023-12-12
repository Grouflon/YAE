#include "Engine.h"

#include <core/string.h>
#include <core/Program.h>
#include <core/Module.h>
#include <core/yae_sdl.h>

#include <yae/resource.h>
#include <yae/Application.h>
#include <yae/FileWatchSystem.h>
#if YAE_TESTS
#include <yae/test/TestSystem.h>
#endif

#include <imgui/imgui.h>

namespace yae {
Engine* Engine::s_engineInstance = nullptr;

void* ImGuiMemAlloc(size_t _size, void* _userData)
{
    return yae::toolAllocator().allocate(_size);
}

void ImGuiMemFree(void* _ptr, void* _userData)
{
    return yae::toolAllocator().deallocate(_ptr);
}

Engine* Engine::Instance()
{
	// NOTE: the static variable might be reset during hot reload.
	// This is a hard-written way to get it back
	/*if (s_engineInstance == nullptr)
	{
		Module* yaeModule = program().findModule("yae");
		YAE_ASSERT(yaeModule != nullptr);
		s_engineInstance = (Engine*) yaeModule->userData;
	}*/
	return s_engineInstance;
}

Engine::Engine()
{
	YAE_ASSERT(s_engineInstance == nullptr);
	s_engineInstance = this;
}

Engine::~Engine()
{
	YAE_ASSERT(s_engineInstance == this);
	s_engineInstance = nullptr;
}

void Engine::init()
{
	YAE_ASSERT(!m_isInitialized);

	// Init ImGui
	{
    	ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);
	}

	// Init Tests
#if YAE_TESTS
	{
		YAE_ASSERT(m_testSystem == nullptr);
		m_testSystem = toolAllocator().create<TestSystem>();
		m_testSystem->init();
		m_testSystem->runAllTests();
	}
#endif

	{
		YAE_ASSERT(m_fileWatchSystem == nullptr);
		m_fileWatchSystem = defaultAllocator().create<FileWatchSystem>();
		m_fileWatchSystem->init();
	}

	m_isInitialized = true;
}

void Engine::shutdown()
{
	YAE_ASSERT(m_isInitialized);
	m_isInitialized = false;

	{
		m_fileWatchSystem->shutdown();
		defaultAllocator().destroy(m_fileWatchSystem);
		m_fileWatchSystem = nullptr;
	}

	#if YAE_TESTS
	{
		m_testSystem->shutdown();
		toolAllocator().destroy(m_testSystem);
		m_testSystem = nullptr;
	}
#endif

	// ImGui shutdown
	{
		ImGui::SetAllocatorFunctions(nullptr, nullptr, nullptr);
	}
}

void Engine::beforeReload()
{
	YAE_ASSERT(s_engineInstance == this);
	s_engineInstance = nullptr;

	for (Application* application : m_applications)
	{
		m_applicationStack.push_back(application);
		application->beforeReload();
		m_applicationStack.pop_back();
	}

	m_fileWatchSystem->pauseAllWatchers();
}

void Engine::afterReload()
{
	YAE_ASSERT(s_engineInstance == nullptr);
	s_engineInstance = this;

	m_fileWatchSystem->resumeAllWatchers();

	for (Application* application : m_applications)
	{
		m_applicationStack.push_back(application);
		application->afterReload();
		m_applicationStack.pop_back();
	}
}

void Engine::startApplication(Application* _application)
{
	YAE_ASSERT(m_isInitialized);
	YAE_ASSERT(_application != nullptr);
	YAE_ASSERT(m_applications.find(_application) == nullptr);

	m_applications.push_back(_application);
	m_applicationStack.push_back(_application);
	_application->_start();
	m_applicationStack.pop_back();	
}

void Engine::stopApplication(Application* _application)
{
	YAE_ASSERT(m_isInitialized);
	YAE_ASSERT(_application != nullptr);
	if (!_application->isRunning())
		return;

	auto it = m_applications.find(_application);
	YAE_ASSERT(it != nullptr);
	YAE_ASSERT(currentApplication() != _application);

	m_applicationStack.push_back(_application);
	_application->_stop();
	m_applicationStack.pop_back();	
	m_applications.erase(it);
}

void Engine::update()
{
	YAE_ASSERT(m_isInitialized);
	DataArray<Application*> tempApplications(m_applications, &scratchAllocator());

	// Events
	{
		auto getWindowId = [](const SDL_Event& _event)
		{
			switch(_event.type)
			{
			case SDL_WINDOWEVENT:
				return _event.window.windowID;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				return _event.key.windowID;

			case SDL_TEXTEDITING:
				return _event.edit.windowID;

			case SDL_TEXTEDITING_EXT:
				return _event.editExt.windowID;

			case SDL_TEXTINPUT:
				return _event.text.windowID;

			case SDL_MOUSEMOTION:
				return _event.motion.windowID;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				return _event.button.windowID;

			case SDL_MOUSEWHEEL:
				return _event.wheel.windowID;

			case SDL_FINGERMOTION:
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
				return _event.tfinger.windowID;

			case SDL_DROPBEGIN:
			case SDL_DROPFILE:
			case SDL_DROPTEXT:
			case SDL_DROPCOMPLETE:
				return _event.drop.windowID;

			case SDL_USEREVENT:
				return _event.user.windowID;

			default:
				return ~0u;
			}
		};

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			YAE_VERBOSEF_CAT("SDL", "SDL event -> type=0x%04x time=%d", event.type, event.common.timestamp);

			u32 windowId = getWindowId(event);
			for (Application* application : tempApplications)
			{
				if (windowId == ~0u || windowId == SDL_GetWindowID(application->m_window))
				{
					application->_pushEvent(event);
				}
			}
		}
	}

	// Update
	for (Application* application : tempApplications)
	{
		if (!application->isRunning())
			continue;

		m_applicationStack.push_back(application);
		application->_doFrame();
		m_applicationStack.pop_back();

		if (application->m_isStopRequested)
		{
			stopApplication(application);
		}
	}

	// Exit
	bool runningApplications = false;
	for (Application* application : m_applications)
	{
		runningApplications = runningApplications || application->isRunning();
	}
	if (!runningApplications)
	{
		program().requestExit();
	}
}

Application* Engine::currentApplication()
{
	return m_applicationStack.empty() ? nullptr : m_applicationStack.back();
}

FileWatchSystem& Engine::fileWatchSystem()
{
	YAE_ASSERT(m_fileWatchSystem != nullptr);
	return *m_fileWatchSystem;
}



} // namespace yae
