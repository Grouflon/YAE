#include "Application.h"

#include <core/filesystem.h>
#include <core/memory.h>
#include <core/Module.h>
#include <core/platform.h>
#include <core/Program.h>
#include <core/serialization/JsonSerializer.h>
#include <core/serialization/serialization.h>
#include <core/string.h>
#include <core/time.h>

#include <yae/Engine.h>
#include <yae/InputSystem.h>
#include <yae/math_3d.h>
#include <yae/ResourceManager.h>
#include <yae/resources/File.h>
#include <yae/SceneSystem.h>

#if YAE_EDITOR
#include <yae/editor/Editor.h>
#endif

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/rendering/renderers/vulkan/VulkanRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <yae/rendering/renderers/opengl/OpenGLRenderer.h>
#endif

#include <imgui/backends/imgui_impl_sdl2.h>
#include <core/yae_sdl.h>

#define YAE_USE_SETTINGS_FILE (YAE_PLATFORM_WEB == 0)

namespace yae {

Application::Application(const char* _name, u32 _width, u32 _height)
	: m_name(_name)
	, m_baseWidth(_width)
	, m_baseHeight(_height)
{

}

Application::~Application()
{

}

void Application::start()
{
	engine().startApplication(this);
}

void Application::stop()
{
	engine().stopApplication(this);
}

void Application::requestStop()
{
	m_isStopRequested = true;
}

void Application::beforeReload()
{
}

void Application::afterReload()
{
}

void Application::_start()
{
	YAE_CAPTURE_FUNCTION();

	m_resourceManager = defaultAllocator().create<ResourceManager>();
	String256 resourcePath = string::format("%s/data/", program().getRootDirectory());
	m_resourceManager->gatherResources(resourcePath.c_str());

	// Init Renderer
#if YAE_PLATFORM_WINDOWS
	m_renderer = defaultAllocator().create<OpenGLRenderer>();
	//m_renderer = defaultAllocator().create<VulkanRenderer>();
#elif YAE_PLATFORM_WEB
	m_renderer = defaultAllocator().create<OpenGLRenderer>();
#endif

	u32 windowFlags = 0;
	windowFlags |= m_renderer->getWindowFlags();
	// windowFlags |= SDL_WINDOWPOS_CENTERED;
	windowFlags |= SDL_WINDOW_RESIZABLE;
	m_renderer->hintWindow();
	m_window = SDL_CreateWindow(m_name.c_str(), 0, 0, m_baseWidth, m_baseHeight, windowFlags);
	YAE_ASSERT(m_window != nullptr);

	YAE_VERBOSE_CAT("application", "Created window");

	// Init Input System
	m_inputSystem = defaultAllocator().create<InputSystem>();
	m_inputSystem->init(m_window);

	// Scene System
	m_sceneSystem = defaultAllocator().create<SceneSystem>();
	m_sceneSystem->init();

	// ImGui
	m_imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(m_imguiContext);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsDark();
	switch(m_renderer->getType())
	{
		case RendererType::Vulkan: 
		{
			YAE_VERIFY(ImGui_ImplSDL2_InitForVulkan(m_window));
		}
		break;
		case RendererType::OpenGL:
		{
			YAE_VERIFY(ImGui_ImplSDL2_InitForOpenGL(m_window, nullptr)); // context is not used in current imgui code
		}
		break;
	}

	// Init renderer
	YAE_VERIFY(m_renderer->init(m_window) == true);

	m_clock.reset();

	loadSettings();

#if YAE_EDITOR
	YAE_ASSERT(m_editor == nullptr);
	m_editor = toolAllocator().create<editor::Editor>();
	m_editor->init();
#endif

	_onStart();

	ImGui::SetCurrentContext(nullptr);
	m_isRunning = true;
}

void Application::_stop()
{
	YAE_CAPTURE_FUNCTION();

	m_isRunning = false;
	_onStop();

	#if YAE_EDITOR
	YAE_ASSERT(m_editor != nullptr);
	m_editor->shutdown();
	toolAllocator().destroy(m_editor);
	m_editor = nullptr;
#endif

	ImGui::SetCurrentContext(m_imguiContext);
	renderer().waitIdle();

	m_renderer->shutdown();
	defaultAllocator().destroy(m_renderer);
	m_renderer = nullptr;

	ImGui_ImplSDL2_Shutdown();
	ImGui::SetCurrentContext(nullptr);
	ImGui::DestroyContext(m_imguiContext);
	m_imguiContext = nullptr;

	m_sceneSystem->shutdown();
	defaultAllocator().destroy(m_sceneSystem);
	m_sceneSystem = nullptr;

	m_inputSystem->shutdown();
	defaultAllocator().destroy(m_inputSystem);
	m_inputSystem = nullptr;

	SDL_DestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("application", "Destroyed window");

	m_resourceManager->flushResources();
	defaultAllocator().destroy(m_resourceManager);
	m_resourceManager = nullptr;
}

void Application::_pushEvent(const SDL_Event& _event)
{
	m_events.push_back(_event);
}

void Application::_doFrame()
{
	YAE_CAPTURE_FUNCTION();

	ImGui::SetCurrentContext(m_imguiContext);

	Time frameTime = m_clock.reset();
	m_dt = frameTime.asSeconds();
	m_time += m_dt;
	
	{
		YAE_CAPTURE_SCOPE("beginFrame");

		m_inputSystem->beginFrame();

		// Events
		{
			for (const SDL_Event& event : m_events)
		    {
		    	switch (event.type)
		    	{
		    		case SDL_QUIT:
		    		{
		    			requestStop();
		    		}
		    		break;

		    		case SDL_WINDOWEVENT:
		    		{
		    			switch (event.window.event)
		    			{
		    				case SDL_WINDOWEVENT_MOVED:
		    				{
		    					// event->window.data1 > x
		    					// event->window.data2 > y
		    					_requestSaveSettings();
		    				}
		    				break;

		    				case SDL_WINDOWEVENT_RESIZED:
		    				{
		    					m_renderer->notifyFrameBufferResized(event.window.data1, event.window.data2);
								_requestSaveSettings();
		    				}
		    				break;
		    			}

		    		}
		    		break;

		    		default:
		    		break;
		    	}

		    	ImGui_ImplSDL2_ProcessEvent(&event);
		    	m_inputSystem->processEvent(event);
		    }
		    m_events.clear();
		}

		m_renderer->beginFrame();
		ImGui_ImplSDL2_NewFrame(m_window);
		ImGui::NewFrame();
	}

	_onUpdate(m_dt);

#if YAE_EDITOR
	m_editor->update(m_dt);
#endif

    // Rendering
	ImGui::Render();
    m_renderer->render();
	m_renderer->endFrame();

	// Settings
	if (m_saveSettingsRequested)
	{
		saveSettings();
		m_saveSettingsRequested = false;
	}

	// Reload changed resources
	m_resourceManager->reloadChangedResources();
}

bool Application::isRunning() const
{
	return m_isRunning;
}

const char* Application::getName() const
{
	return m_name.c_str();
}

InputSystem& Application::input() const
{
	YAE_ASSERT(m_inputSystem != nullptr);
	return *m_inputSystem;
}

SceneSystem& Application::sceneSystem() const
{
	YAE_ASSERT(m_sceneSystem != nullptr);
	return *m_sceneSystem;
}

Renderer& Application::renderer() const
{
	YAE_ASSERT(m_renderer != nullptr);
	return *m_renderer;
}

ResourceManager& Application::resourceManager() const
{
	YAE_ASSERT(m_resourceManager != nullptr);
	return *m_resourceManager;
}

void* Application::getUserData(const char* _name) const
{
	StringHash hash = StringHash(_name);
	void*const* userDataPointer = m_userData.get(hash);
	return userDataPointer != nullptr ? *userDataPointer : nullptr;
}

void Application::setUserData(const char* _name, void* _userData)
{
	StringHash hash = StringHash(_name);
	m_userData.set(hash, _userData);
}

static String getSettingsFilePath(const Application* _app)
{
	String path = filesystem::normalizePath(string::format("%s/%s_settings.json", program().getSettingsDirectory(), _app->getName()).c_str());
	// @TODO(remi): Let's find a better way to sanitize a string for paths at some point
	path.replace(" ", "");
	path.replace("|", "");
	return path;
}

void Application::loadSettings()
{
#if YAE_USE_SETTINGS_FILE
	String filePath = getSettingsFilePath(this);
	FileReader reader(filePath.c_str(), &scratchAllocator());
	if (!reader.load())
	{
		// No settings file.
		// Apply some initial values
		setWindowPosition(0, 30);

		return;
	}

	JsonSerializer serializer(&scratchAllocator());
	if (!serializer.parseSourceData(reader.getContent(), reader.getContentSize()))
	{
		YAE_ERRORF_CAT("application", "Failed to parse json settings file \"%s\"", filePath.c_str());
		return;	
	}
	serializer.beginRead();
	YAE_VERIFY(serializer.beginSerializeObject());
	if (!_onSerialize(&serializer))
	{
		YAE_ERRORF_CAT("application", "Failed to serialize application.");
	}
	YAE_VERIFY(serializer.endSerializeObject());
	serializer.endRead();

	YAE_LOGF_CAT("application", "Loaded application settings from \"%s\"", filePath.c_str());
#endif
}

void Application::saveSettings()
{
#if YAE_USE_SETTINGS_FILE
	JsonSerializer serializer(&scratchAllocator());
	serializer.beginWrite();
	YAE_VERIFY(serializer.beginSerializeObject());
	if (!_onSerialize(&serializer))
	{
		YAE_ERRORF_CAT("application", "Failed to serialize application.");
	}
	YAE_VERIFY(serializer.endSerializeObject());
	serializer.endWrite();

	String filePath = getSettingsFilePath(this);
	FileHandle file(filePath.c_str());
	if (!file.open(FileHandle::OPENMODE_WRITE))
	{
		YAE_ERRORF_CAT("application", "Failed to open \"%s\" for write", filePath.c_str());
		return;
	}
	if (!file.write(serializer.getWriteData(), serializer.getWriteDataSize()))
	{
		YAE_ERRORF_CAT("application", "Failed to write into \"%s\"", filePath.c_str());
		return;
	}
	file.close();

	YAE_VERBOSEF_CAT("application", "Saved application settings to \"%s\"", filePath.c_str());
#endif
}

float Application::getDeltaTime() const
{
	return m_dt;
}

float Application::getTime() const
{
	return m_time;
}

void Application::setWindowSize(i32 _width, i32 _height)
{
	YAE_ASSERT(m_window != nullptr);
	SDL_SetWindowSize(m_window, _width, _height);
	m_renderer->notifyFrameBufferResized(_width, _height);
}

void Application::setWindowSize(const Vector2& _size)
{
	setWindowSize(_size.x, _size.y);
}

void Application::getWindowSize(i32* _outWidth, i32* _outHeight) const
{
	YAE_ASSERT(m_window != nullptr);
	SDL_GetWindowSize(m_window, _outWidth, _outHeight);
}

Vector2 Application::getWindowSize() const
{
	i32 w, h;
	getWindowSize(&w, &h);
	return Vector2(w, h);
}

void Application::setWindowPosition(i32 _x, i32 _y)
{
	YAE_ASSERT(m_window != nullptr);
	SDL_SetWindowPosition(m_window, _x, _y);
}

void Application::setWindowPosition(const Vector2& _position)
{
	setWindowPosition(_position.x, _position.y);
}

void Application::getWindowPosition(i32* _outX, i32* _outY) const
{
	YAE_ASSERT(m_window != nullptr);
	SDL_GetWindowPosition(m_window, _outX, _outY);
}

Vector2 Application::getWindowPosition() const
{
	i32 x, y;
	getWindowSize(&x, &y);
	return Vector2(x, y);
}

void Application::_onStart()
{

}

void Application::_onStop()
{

}

void Application::_onReload()
{

}

void Application::_onUpdate(float _dt)
{

}

struct WindowSettings
{
	i32 windowWidth = -1;
	i32 windowHeight = -1;
	i32 windowX = -1;
	i32 windowY = -1;
};

bool Application::_onSerialize(Serializer* _serializer)
{
	WindowSettings settings;
	getWindowSize(&settings.windowWidth, &settings.windowHeight);
	getWindowPosition(&settings.windowX, &settings.windowY);

	if (!serialization::serializeMirrorType(_serializer, settings, "application"))
		return false;

	if (settings.windowWidth > 0 && settings.windowHeight > 0)
	{
		setWindowSize(settings.windowWidth, settings.windowHeight);
	}
	// @TODO(remi): We need to find a better way to discriminate uninitialized values than that
	if (settings.windowX != -1 && settings.windowY != -1)
	{
		setWindowPosition(settings.windowX, settings.windowY);
	}
	return true;
}

void Application::_requestSaveSettings()
{
	m_saveSettingsRequested = true;
}

} // namespace yae

MIRROR_CLASS(yae::WindowSettings)
(
	MIRROR_MEMBER(windowWidth);
	MIRROR_MEMBER(windowHeight);
	MIRROR_MEMBER(windowX);
	MIRROR_MEMBER(windowY);
);
