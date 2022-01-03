#include "application.h"

#include <yae/platform.h>
#include <yae/program.h>
#include <yae/time.h>
#include <yae/vulkan/VulkanRenderer.h>
#include <yae/vulkan/im3d_impl_vulkan.h>
#include <yae/input.h>
#include <yae/memory.h>
#include <yae/math.h>

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <im3d/im3d.h>
#include <imgui/backends/imgui_impl_glfw.h>

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

	// Init GLFW
	{
		YAE_CAPTURE_SCOPE("glfwInit");

		int result = glfwInit();
		YAE_ASSERT(result == GLFW_TRUE);
		YAE_VERBOSE_CAT("glfw", "Initialized glfw");
	}

	// Init Window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
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

	// Init Vulkan
	m_vulkanRenderer = defaultAllocator().create<VulkanRenderer>();

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	m_vulkanRenderer->init(m_window, enableValidationLayers);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	m_imgui = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	{
		bool ret = ImGui_ImplGlfw_InitForVulkan(m_window, true);
		YAE_ASSERT(ret);
	}

	// Setup Im3d
	m_im3d = toolAllocator().create<Im3d::Context>();
	Im3d::SetContext(*m_im3d);

	m_vulkanRenderer->initImGui();
	m_vulkanRenderer->initIm3d();

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
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		im3d_FrameData frameData;
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

		im3d_NewFrame(frameData);
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
    	ImGui::End();
    }

	//ImGui::ShowDemoWindow(&s_showDemoWindow);

    // Rendering
	VkCommandBuffer commandBuffer = m_vulkanRenderer->beginFrame();
	m_vulkanRenderer->beginSwapChainRenderPass(commandBuffer);	

    // Mesh
    m_vulkanRenderer->drawCommands(commandBuffer);

	// Im3d
	{
		Im3d::EndFrame();
		m_vulkanRenderer->drawIm3d(commandBuffer);
	}

	// ImGui
	{
		ImGui::Render();
		ImDrawData* imguiDrawData = ImGui::GetDrawData();
		const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
		if (!isMinimized)
		{
			m_vulkanRenderer->drawImGui(imguiDrawData, commandBuffer);
		}	
	}

	// End Frame
	m_vulkanRenderer->endSwapChainRenderPass(commandBuffer);
	m_vulkanRenderer->endFrame();

	return true;
}

void Application::shutdown()
{
	YAE_CAPTURE_FUNCTION();
	
	program().shutdownGame();

	m_vulkanRenderer->shutdownIm3d();
	m_vulkanRenderer->shutdownImGui();

	// @NOTE: Can't unset context, since the setter uses a ref
	toolAllocator().destroy(m_im3d);
	m_im3d = nullptr;

	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imgui);
	m_imgui = nullptr;

	m_vulkanRenderer->shutdown();
	defaultAllocator().destroy<VulkanRenderer>(m_vulkanRenderer);
	m_vulkanRenderer = nullptr;

	m_inputSystem->shutdown();
	defaultAllocator().destroy<InputSystem>(m_inputSystem);
	m_inputSystem = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");

	glfwTerminate();
	YAE_VERBOSE_CAT("glfw", "Terminated glfw");
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


VulkanRenderer& Application::renderer() const
{
	YAE_ASSERT(m_vulkanRenderer != nullptr);
	return *m_vulkanRenderer;
}


void Application::_glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height)
{
	Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
	application->m_vulkanRenderer->notifyFrameBufferResized(_width, _height);
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
