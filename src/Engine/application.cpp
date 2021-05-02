#include "Application.h"

#include <platform.h>
#include <profiling.h>
#include <log.h>
#include <yae_time.h>
#include <vulkan/VulkanRenderer.h>
#include <input.h>
#include <game_module.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

namespace yae {

void Application::init(const char* _name, u32 _width, u32 _height, char** _args, int _arg_count)
{
	YAE_CAPTURE_FUNCTION();

	m_name = _name;

	m_exePath = Path(_args[0]);
	YAE_LOG(m_exePath.c_str());

	String workingDirectory = platform::getWorkingDirectory();
	Path workingDirectoryPath(workingDirectory.c_str());
	YAE_LOG(workingDirectoryPath.c_str());

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
	m_window = glfwCreateWindow(_width, _height, m_name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, &Application::_glfw_framebufferSizeCallback);
	glfwSetKeyCallback(m_window, &Application::_glfw_keyCallback);
	glfwSetMouseButtonCallback(m_window, &Application::_glfw_mouseButtonCallback);
	glfwSetScrollCallback(m_window, &Application::_glfw_scrollCallback);

	YAE_ASSERT(m_window);
	YAE_VERBOSE_CAT("glfw", "Created glfw window");

	// Init Input System
	m_inputSystem = context().defaultAllocator->create<InputSystem>();
	m_inputSystem->init(m_window);

	// Init Vulkan
	m_vulkanRenderer = context().defaultAllocator->create<VulkanRenderer>();

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
	m_vulkanRenderer->initImGui();


	initGame();
}

void Application::run()
{
	Clock clock;
	clock.reset();

	//static bool s_showDemoWindow = true;
	while (!glfwWindowShouldClose(m_window))
	{
		YAE_CAPTURE_START("frame");

		{
			YAE_CAPTURE_SCOPE("beginFrame");

			//glfwPollEvents();
			m_inputSystem->update();
			ImGui_ImplGlfw_NewFrame();
			m_vulkanRenderer->beginFrame();
			ImGui::NewFrame();	
		}
		
		if (getInputSystem().isCtrlDown() && getInputSystem().wasKeyJustPressed(GLFW_KEY_R))
		{
			unloadGameAPI();
			loadGameAPI();
		}

		updateGame();

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
	    	auto showAllocatorInfo = [](const char* _name, Allocator* _allocator)
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

	    		formatSize(_allocator->getAllocatedSize(), allocatedSizeBuffer);
	    		formatSize(_allocator->getAllocableSize(), allocableSizeBuffer);

	    		ImGui::Text("%s: %s / %s, %zu allocations",
		    		_name,
		    		allocatedSizeBuffer,
		    		allocableSizeBuffer,
		    		_allocator->getAllocationCount()
		    	);
	    	};

	    	ImGui::Begin("Memory Profiler", &m_showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize);
	    	showAllocatorInfo("Default", context().defaultAllocator);
	    	showAllocatorInfo("Scratch", context().scratchAllocator);
	    	showAllocatorInfo("Tool", context().toolAllocator);
	    	ImGui::End();
	    }
		//ImGui::ShowDemoWindow(&s_showDemoWindow);

		m_vulkanRenderer->drawMesh();

		// Rendering
		ImGui::Render();
		ImDrawData* imguiDrawData = ImGui::GetDrawData();
		const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
		if (!isMinimized)
		{
			m_vulkanRenderer->drawImGui(imguiDrawData);
		}

		m_vulkanRenderer->endFrame();

		watchGameAPI();

		/*Time frameTime = clock.reset();
		printf("%f\n", 1.0 / frameTime.asSeconds64());*/

#if YAE_PROFILING_ENABLED
		context().profiler->update();
#endif

		YAE_CAPTURE_STOP("frame");
	}
	m_vulkanRenderer->waitIdle();
}

void Application::shutdown()
{
	YAE_CAPTURE_FUNCTION();
	
	shutdownGame();

	m_vulkanRenderer->shutdownImGui();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imgui);
	m_imgui = nullptr;

	m_vulkanRenderer->shutdown();
	context().defaultAllocator->destroy<VulkanRenderer>(m_vulkanRenderer);
	m_vulkanRenderer = nullptr;

	m_inputSystem->shutdown();
	context().defaultAllocator->destroy<InputSystem>(m_inputSystem);
	m_inputSystem = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");

	glfwTerminate();
	YAE_VERBOSE_CAT("glfw", "Terminated glfw");
}


InputSystem& Application::getInputSystem() const
{
	return *m_inputSystem;
}


VulkanRenderer& Application::getRenderer() const
{
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
