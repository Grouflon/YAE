#include "Application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/Module.h>
#include <yae/time.h>
#include <yae/memory.h>
#include <yae/string.h>
#include <yae/InputSystem.h>
#include <yae/math_3d.h>
#include <yae/resource.h>
#include <yae/resources/File.h>
#include <yae/filesystem.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/JsonSerializer.h>

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/rendering/renderers/vulkan/VulkanRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <yae/rendering/renderers/opengl/OpenGLRenderer.h>
#endif

#include <imgui/backends/imgui_impl_sdl.h>
#include <yae/yae_sdl.h>

#define YAE_USE_SETTINGS_FILE (YAE_PLATFORM_WEB == 0)

namespace yae {

struct ApplicationSettings
{
	i32 windowWidth = -1;
	i32 windowHeight = -1;
	i32 windowX = -1;
	i32 windowY = -1;

	MIRROR_CLASS_NOVIRTUAL(ApplicationSettings)
	(
		MIRROR_MEMBER(windowWidth)();
		MIRROR_MEMBER(windowHeight)();
		MIRROR_MEMBER(windowX)();
		MIRROR_MEMBER(windowY)();
	);
};

MIRROR_CLASS_DEFINITION(ApplicationSettings);

Application::Application(const char* _name, u32 _width, u32 _height)
	: m_name(_name)
	, m_baseWidth(_width)
	, m_baseHeight(_height)
{

}

Application::~Application()
{

}

void Application::init(char** _args, int _argCount)
{
	YAE_CAPTURE_FUNCTION();

	m_resourceManager = defaultAllocator().create<ResourceManager>();

	// Init Renderer
#if YAE_PLATFORM_WINDOWS
	m_renderer = defaultAllocator().create<OpenGLRenderer>();
	//m_renderer = defaultAllocator().create<VulkanRenderer>();
#elif YAE_PLATFORM_WEB
	m_renderer = defaultAllocator().create<OpenGLRenderer>();
#endif

	u32 windowFlags = 0;
	windowFlags |= m_renderer->getWindowFlags();
	m_renderer->hintWindow();
	m_window = SDL_CreateWindow(m_name.c_str(), 0, 0, m_baseWidth, m_baseHeight, windowFlags);
	YAE_ASSERT(m_window != nullptr);

	YAE_VERBOSE_CAT("application", "Created window");

	// Init Input System
	m_inputSystem = defaultAllocator().create<InputSystem>();
	m_inputSystem->init(m_window);

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

	ImGui::SetCurrentContext(nullptr);
}

void Application::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	ImGui::SetCurrentContext(m_imguiContext);
	renderer().waitIdle();

	m_renderer->shutdown();
	defaultAllocator().destroy(m_renderer);
	m_renderer = nullptr;

	ImGui_ImplSDL2_Shutdown();
	ImGui::SetCurrentContext(nullptr);
	ImGui::DestroyContext(m_imguiContext);
	m_imguiContext = nullptr;

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

bool Application::doFrame()
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
		// @TODO: This won't work for several applications. We need to escalate that to program and dispatch events by window
		{
			SDL_Event event;
		    while (SDL_PollEvent(&event))
		    {
		    	YAE_VERBOSEF_CAT("SDL", "SDL event -> type=0x%04x time=%d", event.type, event.common.timestamp);

		    	switch (event.type)
		    	{
		    		case SDL_QUIT:
		    		{
		    			requestExit();
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
		}

		m_renderer->beginFrame();
		ImGui_ImplSDL2_NewFrame(m_window);
		ImGui::NewFrame();
	}

	for (Module* module : program().getModules())
	{
		if (module->updateApplicationFunction != nullptr)
		{
			module->updateApplicationFunction(this, m_dt);
		}
	}

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

	return isRunning();
}

void Application::requestExit()
{
	m_isRunning = false;
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

static bool serializeSettings(Serializer* _serializer, ApplicationSettings* _settings)
{
	YAE_VERIFY(_serializer->beginSerializeObject());

	serialization::serializeMirrorType(_serializer, *_settings, "application");
	for (Module* module : program().getModules())
	{
		if (module->onSerializeApplicationSettingsFunction != nullptr)
		{
			YAE_VERIFY(module->onSerializeApplicationSettingsFunction(&app(), _serializer));
		}
	}

	YAE_VERIFY(_serializer->endSerializeObject());
	return true;
}

void Application::loadSettings()
{
#if YAE_USE_SETTINGS_FILE
	String filePath = getSettingsFilePath(this);
	File* settingsFile = resource::findOrCreateFile<File>(filePath.c_str());
	if (!settingsFile->load())
	{
		YAE_ERRORF_CAT("application", "Failed to load settings file \"%s\"", filePath.c_str());
		settingsFile->release();
		return;
	}

	JsonSerializer serializer(&scratchAllocator());
	if (!serializer.parseSourceData(settingsFile->getContent(), settingsFile->getContentSize()))
	{
		YAE_ERRORF_CAT("application", "Failed to parse json settings file \"%s\"", filePath.c_str());
		settingsFile->release();
		return;	
	}
	settingsFile->release();

	ApplicationSettings settings;
	serializer.beginRead();
	serializeSettings(&serializer, &settings);
	serializer.endRead();

	if (settings.windowWidth > 0 && settings.windowHeight > 0)
	{
		setWindowSize(settings.windowWidth, settings.windowHeight);
	}
	// @TODO(remi): We need to find a better way to discriminate uninitialized values than that
	if (settings.windowX != -1 && settings.windowY != -1)
	{
		setWindowPosition(settings.windowX, settings.windowY);
	}

	YAE_LOGF_CAT("application", "Loaded application settings from \"%s\"", filePath.c_str());
#endif
}

void Application::saveSettings()
{
#if YAE_USE_SETTINGS_FILE
	ApplicationSettings settings;
	getWindowSize(&settings.windowWidth, &settings.windowHeight);
	getWindowPosition(&settings.windowX, &settings.windowY);

	JsonSerializer serializer(&scratchAllocator());
	serializer.beginWrite();
	serializeSettings(&serializer, &settings);
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

void Application::_requestSaveSettings()
{
	m_saveSettingsRequested = true;
}

} // namespace yae
