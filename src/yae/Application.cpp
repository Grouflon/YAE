#include "Application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/Module.h>
#include <yae/time.h>
#include <yae/memory.h>
#include <yae/string.h>
#include <yae/input.h>
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

#include <imgui/backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

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

	// Init Window
	glfwDefaultWindowHints();
	m_renderer->hintWindow();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(m_baseWidth, m_baseHeight, m_name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);

	YAE_ASSERT(m_window);
	// Setup callbacks
	glfwSetWindowPosCallback(m_window, &Application::_glfw_windowPosCallback);
	glfwSetFramebufferSizeCallback(m_window, &Application::_glfw_framebufferSizeCallback);
	glfwSetKeyCallback(m_window, &Application::_glfw_keyCallback);
	glfwSetMouseButtonCallback(m_window, &Application::_glfw_mouseButtonCallback);
	glfwSetScrollCallback(m_window, &Application::_glfw_scrollCallback);

	YAE_VERBOSE_CAT("glfw", "Created glfw window");

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
			YAE_VERIFY(ImGui_ImplGlfw_InitForVulkan(m_window, true));
		}
		break;
		case RendererType::OpenGL:
		{
			YAE_VERIFY(ImGui_ImplGlfw_InitForOpenGL(m_window, true));
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

	ImGui_ImplGlfw_Shutdown();
	ImGui::SetCurrentContext(nullptr);
	ImGui::DestroyContext(m_imguiContext);
	m_imguiContext = nullptr;

	m_inputSystem->shutdown();
	defaultAllocator().destroy(m_inputSystem);
	m_inputSystem = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");

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

	if (glfwWindowShouldClose(m_window))
		return false;

	{
		YAE_CAPTURE_SCOPE("beginFrame");

		//glfwPollEvents();
		m_inputSystem->update();

		m_renderer->beginFrame();
		ImGui_ImplGlfw_NewFrame();
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

	return true;
}

void Application::requestExit()
{
	glfwSetWindowShouldClose(m_window, true);
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
		glfwSetWindowSize(m_window, settings.windowWidth, settings.windowHeight);
		m_renderer->notifyFrameBufferResized(settings.windowWidth, settings.windowHeight); // @TODO(remi): This should be notified automatically. Maybe we should wrap this call
	}
	// @TODO(remi): We need to find a better way to discriminate uninitialized values than that
	if (settings.windowX != -1 && settings.windowY != -1)
	{
		glfwSetWindowPos(m_window, settings.windowX, settings.windowY);
	}

	YAE_LOGF_CAT("application", "Loaded application settings from \"%s\"", filePath.c_str());
}

void Application::saveSettings()
{
	ApplicationSettings settings;
	glfwGetWindowSize(m_window, &settings.windowWidth, &settings.windowHeight);
	glfwGetWindowPos(m_window, &settings.windowX, &settings.windowY);

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
}

float Application::getDeltaTime() const
{
	return m_dt;
}

float Application::getTime() const
{
	return m_time;
}

void Application::_glfw_windowPosCallback(GLFWwindow* _window, int _x, int _y)
{
	Application* application = reinterpret_cast<class Application*>(glfwGetWindowUserPointer(_window));

	application->_requestSaveSettings();
}

void Application::_glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	Application* application = reinterpret_cast<class Application*>(glfwGetWindowUserPointer(_window));
	application->m_renderer->notifyFrameBufferResized(_width, _height);

	application->_requestSaveSettings();
}

void Application::_glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	Application* Application = reinterpret_cast<class Application*>(glfwGetWindowUserPointer(_window));
	Application->m_inputSystem->notifyKeyEvent(_key, _scancode, _action, _mods);
}

void Application::_glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods)
{
	Application* Application = reinterpret_cast<class Application*>(glfwGetWindowUserPointer(_window));
	Application->m_inputSystem->notifyMouseButtonEvent(_button, _action, _mods);
}

void Application::_glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset)
{
	Application* Application = reinterpret_cast<class Application*>(glfwGetWindowUserPointer(_window));
	Application->m_inputSystem->notifyMouseScrollEvent(_xOffset, _yOffset);
}

void Application::_requestSaveSettings()
{
	m_saveSettingsRequested = true;
}

} // namespace yae
