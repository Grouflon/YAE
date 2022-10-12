#include "application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/time.h>
#include <yae/input.h>
#include <yae/memory.h>
#include <yae/math.h>
#include <yae/ImGuiSystem.h>
#include <yae/Im3dSystem.h>

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/rendering/renderers/vulkan/VulkanRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <yae/rendering/renderers/opengl/OpenGLRenderer.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

namespace yae {

Application::Application(const char* _name, u32 _width, u32 _height)
	: m_name(_name)
	, m_width(_width)
	, m_height(_height)
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

	m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, &Application::_glfw_framebufferSizeCallback);
	glfwSetKeyCallback(m_window, &Application::_glfw_keyCallback);
	glfwSetMouseButtonCallback(m_window, &Application::_glfw_mouseButtonCallback);
	glfwSetScrollCallback(m_window, &Application::_glfw_scrollCallback);

	YAE_ASSERT(m_window);
	YAE_VERBOSE_CAT("glfw", "Created glfw window");

	// Init Input System
	m_inputSystem = defaultAllocator().create<InputSystem>();
	m_inputSystem->init(m_window);

	// Init renderer
	YAE_VERIFY(m_renderer->init(m_window) == true);

	// Setup Dear ImGui context
	m_imGuiSystem = defaultAllocator().create<ImGuiSystem>();
	m_imGuiSystem->init(m_window, m_renderer);

	// Setup Im3d
	m_im3dSystem = defaultAllocator().create<Im3dSystem>();
	m_im3dSystem->init();

	/*m_im3d = toolAllocator().create<Im3d::Context>();
	Im3d::SetContext(*m_im3d);

	m_renderer->initIm3d();*/

	m_clock.reset();

	program().initGame();
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

		// This is weird that this is here, it should be just before rendering
		// But im3d seems to need them when its newframe call is done...
		Matrix4 view = _computeCameraView();
		Matrix4 proj = _computeCameraProj();
		renderer().setViewProjectionMatrix(view, proj);

		//glfwPollEvents();
		m_inputSystem->update();

		m_imGuiSystem->newFrame();

		Im3dCamera im3dCamera = {};
		im3dCamera.position = m_cameraPosition;
		im3dCamera.direction = m_cameraRotation * Vector3::FORWARD;
		im3dCamera.view = view;
		im3dCamera.projection = proj;
		im3dCamera.fov = m_cameraFov * D2R;
		im3dCamera.orthographic = false;
		m_im3dSystem->newFrame(dt, im3dCamera);
	}

	program().updateGame(dt);

	if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
        	if (ImGui::MenuItem("Exit"))
        	{
        		glfwSetWindowShouldClose(m_window, true);
        	}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Profiling"))
        {
        	ImGui::MenuItem("Memory", NULL, &m_showMemoryProfiler);
        	ImGui::MenuItem("Frame rate", NULL, &m_showFrameRate);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (m_showMemoryProfiler)
    {
    	auto showAllocatorInfo = [](const char* _name, const Allocator& _allocator)
    	{
    		char allocatedSizeBuffer[32];
	    	char allocableSizeBuffer[32];
	    	auto formatSize = [](size_t _size, char _buf[32])
	    	{
	    		if (_size == Allocator::SIZE_NOT_TRACKED)
	    		{
	    			snprintf(_buf, 31, "???");
	    		}
	    		else
	    		{
	    			const char* units[] =
	    			{
	    				"b",
	    				"Kb",
	    				"Mb",
	    				"Gb",
	    				"Tb"
	    			};

	    			u8 unit = 0;
	    			u32 mod = 1024 * 10;
	    			while (_size > mod)
	    			{
	    				_size = _size / mod;
	    				++unit;
	    			}

	    			snprintf(_buf, 31, "%zu %s", _size, units[unit]);
	    		}
	    	};

    		formatSize(_allocator.getAllocatedSize(), allocatedSizeBuffer);
    		formatSize(_allocator.getAllocableSize(), allocableSizeBuffer);

    		ImGui::Text("%s: %s / %s, %zu allocations",
	    		_name,
	    		allocatedSizeBuffer,
	    		allocableSizeBuffer,
	    		_allocator.getAllocationCount()
	    	);
    	};

    	ImGui::Begin("Memory Profiler", &m_showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize);
    	showAllocatorInfo("Default", defaultAllocator());
    	showAllocatorInfo("Scratch", scratchAllocator());
    	showAllocatorInfo("Tool", toolAllocator());
    	showAllocatorInfo("Malloc", mallocAllocator());
    	ImGui::End();
    }

    if (m_showFrameRate)
    {
    	ImGui::Begin("Frame Rate", &m_showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs);
    	ImGui::Text("dt: %.2f ms", dt*1000.f);
    	ImGui::Text("%.2f fps", dt != 0.f ? (1.f/dt) : 0.f);
    	ImGui::End();
    }

	//ImGui::ShowDemoWindow(&s_showDemoWindow);

    // Rendering
	FrameHandle frameHandle = m_renderer->beginFrame();

    // Mesh
    m_renderer->drawCommands(frameHandle);

	// Im3d
	{
		m_im3dSystem->endFrame();
		m_im3dSystem->render(frameHandle);
		//Im3d::EndFrame();
		//m_renderer->drawIm3d(commandBuffer);
	}

	// ImGui
	m_imGuiSystem->render(frameHandle);

	// End Frame
	m_renderer->endFrame();

	return true;
}

void Application::shutdown()
{
	YAE_CAPTURE_FUNCTION();
	
	program().shutdownGame();

	/*m_renderer->shutdownIm3d();

	// @NOTE: Can't unset context, since the setter uses a ref
	toolAllocator().destroy(m_im3d);
	m_im3d = nullptr;*/
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


void Application::requestExit()
{
	glfwSetWindowShouldClose(m_window, true);
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

void Application::_glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
	application->m_renderer->notifyFrameBufferResized(_width, _height);
}


void Application::_glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
	application->m_inputSystem->notifyKeyEvent(_key, _scancode, _action, _mods);
}


void Application::_glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods)
{
	Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
	application->m_inputSystem->notifyMouseButtonEvent(_button, _action, _mods);
}


void Application::_glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset)
{
	Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
	application->m_inputSystem->notifyMouseScrollEvent(_xOffset, _yOffset);
}

Matrix4 Application::_computeCameraView() const
{
	Matrix4 cameraTransform = makeTransformMatrix(m_cameraPosition, m_cameraRotation, Vector3::ONE);
	Matrix4 view = inverse(cameraTransform);
	return view;
}

Matrix4 Application::_computeCameraProj() const
{
	Vector2 viewportSize = renderer().getFrameBufferSize();
	float fov = m_cameraFov * D2R;
	float aspectRatio = viewportSize.x / viewportSize.y;
	Matrix4 proj = glm::perspective(fov, aspectRatio, .1f, 100.f);
	proj[1][1] *= -1.f;
	return proj;
}

} // namespace yae
