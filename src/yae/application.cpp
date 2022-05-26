#include "application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/time.h>
#include <yae/input.h>
#include <yae/memory.h>
#include <yae/math.h>
#include <yae/ImGuiSystem.h>

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/renderers/vulkan/VulkanRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <yae/renderers/opengl/OpenGLRenderer.h>
#endif

#include <GLFW/glfw3.h>
#include <im3d/im3d.h>
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
	m_renderer->init(m_window);

	// Setup Dear ImGui context
	m_imGuiSystem = defaultAllocator().create<ImGuiSystem>();
	m_imGuiSystem->init(m_window, m_renderer);

	// Setup Im3d
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
	//printf("%f\n", 1.0 / frameTime.asSeconds64());

	if (glfwWindowShouldClose(m_window))
		return false;

	{
		YAE_CAPTURE_SCOPE("beginFrame");

		// This is weird that this is here, it should be just before rendering
		Vector2 viewportSize = renderer().getFrameBufferSize();
		float fov = 45.f * D2R;
		float aspectRatio = viewportSize.x / viewportSize.y;
		//Vector3 position = Vector3(2.f, 2.f, 2.f);
		//Vector3 target = glm::vec3(0.f, 0.f, 0.f);
		Matrix4 cameraTransform = makeTransformMatrix(m_cameraPosition, m_cameraRotation, Vector3::ONE);
		Matrix4 view = inverse(cameraTransform);
		//Matrix4 view = glm::lookAt(position, target, glm::vec3(0.f, 1.f, 0.f));
		Matrix4 proj = glm::perspective(fov, aspectRatio, .1f, 100.f);
		proj[1][1] *= -1.f;

		renderer().setViewProjectionMatrix(view, proj);

		//glfwPollEvents();
		m_inputSystem->update();

		m_imGuiSystem->newFrame();

		/*im3d_FrameData frameData;
		frameData.deltaTime = dt;
		frameData.cursorPosition = input().getMousePosition();
		frameData.viewportSize = viewportSize;
		frameData.camera.position = m_cameraPosition;
		frameData.camera.direction = m_cameraRotation * Vector3::FORWARD;
		frameData.camera.view = view;
		frameData.camera.projection = proj;
		frameData.camera.fov = fov;
		frameData.camera.orthographic = false;

		frameData.actionKeyStates[Im3d::Action_Select] = input().isMouseButtonDown(0);

		im3d_NewFrame(frameData);*/
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

	//ImGui::ShowDemoWindow(&s_showDemoWindow);

    // Rendering
	FrameHandle frameHandle = m_renderer->beginFrame();

    // Mesh
    m_renderer->drawCommands(frameHandle);

	// Im3d
	/*{
		Im3d::EndFrame();
		m_renderer->drawIm3d(commandBuffer);
	}*/

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

} // namespace yae
