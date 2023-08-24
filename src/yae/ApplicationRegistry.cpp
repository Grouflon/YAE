#include "ApplicationRegistry.h"

#include <core/program.h>

#include <yae/Application.h>

namespace yae {

static ApplicationRegistry* s_singleton = nullptr;


ApplicationRegistry::ApplicationRegistry()
{
	YAE_ASSERT(s_singleton == nullptr);
	s_singleton = this;
}

ApplicationRegistry::~ApplicationRegistry()
{
	YAE_ASSERT(s_singleton != nullptr);
	YAE_ASSERT(m_applications.empty());
	s_singleton = nullptr;
}

void ApplicationRegistry::StartApplication(Application* _application)
{
	YAE_ASSERT(_application != nullptr);
	YAE_ASSERT(s_singleton->m_applications.find(_application) == nullptr);

	s_singleton->m_applications.push_back(_application);
	s_singleton->m_applicationStack.push_back(_application);
	_application->_start();
	s_singleton->m_applicationStack.pop_back();	
}

void ApplicationRegistry::StopApplication(Application* _application)
{
	YAE_ASSERT(_application != nullptr);
	if (!_application->isRunning())
		return;

	auto it = s_singleton->m_applications.find(_application);
	YAE_ASSERT(it != nullptr);
	YAE_ASSERT(CurrentApplication() != _application);

	s_singleton->m_applicationStack.push_back(_application);
	_application->_stop();
	s_singleton->m_applicationStack.pop_back();	
	s_singleton->m_applications.erase(it);
}

void ApplicationRegistry::Update()
{
	DataArray<Application*> tempApplications(s_singleton->m_applications, &scratchAllocator());

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

		s_singleton->m_applicationStack.push_back(application);
		application->_doFrame();
		s_singleton->m_applicationStack.pop_back();

		if (application->m_isStopRequested)
		{
			StopApplication(application);
		}
	}

	// Exit
	bool runningApplications = false;
	for (Application* application : s_singleton->m_applications)
	{
		runningApplications = runningApplications || application->isRunning();
	}
	if (!runningApplications)
	{
		program().requestExit();
	}
}

Application* ApplicationRegistry::CurrentApplication()
{
	return s_singleton->m_applicationStack.empty() ? nullptr : s_singleton->m_applicationStack.back();
}

} // namespace yae
