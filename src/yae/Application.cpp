#include "Application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/Module.h>
#include <yae/time.h>
#include <yae/memory.h>
#include <yae/input.h>
#include <yae/math_3d.h>
#include <yae/ImGuiSystem.h>
#include <yae/Im3dSystem.h>
#include <yae/resources/FileResource.h>
#include <yae/filesystem.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/JsonSerializer.h>

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/rendering/renderers/vulkan/VulkanRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <yae/rendering/renderers/opengl/OpenGLRenderer.h>
#endif

#include <GLFW/glfw3.h>

namespace yae {

struct ApplicationSettings
{
	i32 windowWidth = -1;
	i32 windowHeight = -1;
	i32 windowX = -1;
	i32 windowY = -1;

	float cameraPosition[3] = {};
	float cameraRotation[4] = {0.f, 0.f, 0.f, 1.f};

	MIRROR_CLASS_NOVIRTUAL(ApplicationSettings)
	(
		MIRROR_MEMBER(windowWidth)();
		MIRROR_MEMBER(windowHeight)();
		MIRROR_MEMBER(windowX)();
		MIRROR_MEMBER(windowY)();
		MIRROR_MEMBER(cameraPosition)();
		MIRROR_MEMBER(cameraRotation)();
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
	YAE_VERBOSE_CAT("glfw", "Created glfw window");

	// Init Input System
	m_inputSystem = defaultAllocator().create<InputSystem>();
	m_inputSystem->init(m_window);

	// Init renderer
	YAE_VERIFY(m_renderer->init(m_window) == true);

	m_clock.reset();

	loadSettings();

	// Setup callbacks
	glfwSetWindowPosCallback(m_window, &Application::_glfw_windowPosCallback);
	glfwSetFramebufferSizeCallback(m_window, &Application::_glfw_framebufferSizeCallback);
	glfwSetKeyCallback(m_window, &Application::_glfw_keyCallback);
	glfwSetMouseButtonCallback(m_window, &Application::_glfw_mouseButtonCallback);
	glfwSetScrollCallback(m_window, &Application::_glfw_scrollCallback);

	// Setup Dear ImGui context (depends on glfw callbacks being already bound, therefore is after)
	m_imGuiSystem = defaultAllocator().create<ImGuiSystem>();
	m_imGuiSystem->init(m_window, m_renderer);

	// Setup Im3d
	m_im3dSystem = defaultAllocator().create<Im3dSystem>();
	m_im3dSystem->init();
}

void Application::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	renderer().waitIdle();

	m_im3dSystem->shutdown();
	defaultAllocator().destroy(m_im3dSystem);
	m_im3dSystem = nullptr;

	m_imGuiSystem->shutdown();
	defaultAllocator().destroy(m_imGuiSystem);
	m_imGuiSystem = nullptr;

	m_renderer->shutdown();
	defaultAllocator().destroy(m_renderer);
	m_renderer = nullptr;

	m_inputSystem->shutdown();
	defaultAllocator().destroy(m_inputSystem);
	m_inputSystem = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");
}

bool Application::doFrame()
{
	YAE_CAPTURE_FUNCTION();

	Time frameTime = m_clock.reset();
	float dt = frameTime.asSeconds();

	if (glfwWindowShouldClose(m_window))
		return false;

	{
		YAE_CAPTURE_SCOPE("beginFrame");

		//glfwPollEvents();
		m_inputSystem->update();

		m_imGuiSystem->newFrame();

		// This is weird that this is here, it should be just before rendering
		// But im3d seems to need them when its newframe call is done...
		Matrix4 view = _computeCameraView();
		Matrix4 proj = _computeCameraProj();
		renderer().setViewProjectionMatrix(view, proj);

		Im3dCamera im3dCamera = {};
		im3dCamera.position = m_cameraPosition;
		im3dCamera.direction = math::rotate(m_cameraRotation, Vector3::FORWARD);
		im3dCamera.view = view;
		im3dCamera.projection = proj;
		im3dCamera.fov = m_cameraFov * D2R;
		im3dCamera.orthographic = false;
		m_im3dSystem->newFrame(dt, im3dCamera);
	}

	for (Module* module : program().getModules())
	{
		if (module->updateApplicationFunction != nullptr)
		{
			module->updateApplicationFunction(this, dt);
		}
	}

    // Rendering
    OpenGLRenderer* renderer = (OpenGLRenderer*)m_renderer;

    renderer->beginRenderTarget();
    m_renderer->drawCommands(nullptr);
	m_im3dSystem->render(nullptr);
    renderer->endRenderTarget();

	renderer->beginFrame();
	renderer->drawRenderTarget();
	m_imGuiSystem->render(nullptr);
	m_renderer->endFrame();

	// Settings
	if (m_saveSettingsRequested)
	{
		saveSettings();
		m_saveSettingsRequested = false;
	}

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

ImGuiSystem& Application::imguiSystem() const
{
	YAE_ASSERT(m_imGuiSystem != nullptr);
	return *m_imGuiSystem;
}

Vector3 Application::getCameraPosition() const
{
	return m_cameraPosition;
}

Quaternion Application::getCameraRotation() const
{
	return m_cameraRotation;
}

void Application::setCameraPosition(const Vector3& _position)
{
	if (_position == m_cameraPosition)
		return;

	m_cameraPosition = _position;
	_requestSaveSettings();
}

void Application::setCameraRotation(const Quaternion& _rotation)
{
	if (_rotation == m_cameraRotation)
		return;

	m_cameraRotation = _rotation;
	_requestSaveSettings();
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
	FileResource* settingsFile = findOrCreateResource<FileResource>(filePath.c_str());
	if (!settingsFile->useLoad())
	{
		YAE_ERRORF_CAT("application", "Failed to load settings file \"%s\"", filePath.c_str());
		settingsFile->releaseUnuse();
		return;
	}

	JsonSerializer serializer(&scratchAllocator());
	if (!serializer.parseSourceData(settingsFile->getContent(), settingsFile->getContentSize()))
	{
		YAE_ERRORF_CAT("application", "Failed to parse json settings file \"%s\"", filePath.c_str());
		settingsFile->releaseUnuse();
		return;	
	}
	settingsFile->releaseUnuse();

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

	Vector3 cameraPosition;
	Quaternion cameraRotation;
	memcpy(math::data(cameraPosition), settings.cameraPosition, sizeof(settings.cameraPosition));
	memcpy(math::data(cameraRotation), settings.cameraRotation, sizeof(settings.cameraRotation));
	setCameraPosition(cameraPosition);
	setCameraRotation(cameraRotation);
	YAE_LOGF_CAT("application", "Loaded application settings from \"%s\"", filePath.c_str());
}

void Application::saveSettings()
{
	ApplicationSettings settings;
	glfwGetWindowSize(m_window, &settings.windowWidth, &settings.windowHeight);
	glfwGetWindowPos(m_window, &settings.windowX, &settings.windowY);
	memcpy(settings.cameraPosition, math::data(getCameraPosition()), sizeof(settings.cameraPosition));
	memcpy(settings.cameraRotation, math::data(getCameraRotation()), sizeof(settings.cameraRotation));

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

Matrix4 Application::_computeCameraView() const
{
	Matrix4 cameraTransform = Matrix4::FromTransform(m_cameraPosition, m_cameraRotation, Vector3::ONE);
	Matrix4 view = math::inverse(cameraTransform);
	return view;
}

Matrix4 Application::_computeCameraProj() const
{
	Vector2 viewportSize = renderer().getFrameBufferSize();
	YAE_ASSERT(!math::isZero(viewportSize));
	float fov = m_cameraFov * D2R;
	float aspectRatio = viewportSize.x / viewportSize.y;
	Matrix4 proj = Matrix4::FromPerspective(fov, aspectRatio, .1f, 100.f);
	return proj;
}

void Application::_requestSaveSettings()
{
	m_saveSettingsRequested = true;
}

} // namespace yae
