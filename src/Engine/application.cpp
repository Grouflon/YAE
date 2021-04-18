#include "Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include <platform.h>
#include <profiling.h>
#include <log.h>
#include <yae_time.h>
#include <vulkan/VulkanRenderer.h>

typedef void (*GameFunctionPtr)();

namespace yae {

const char* GAME_DLL_PATH = "./data/code/yaeGame.dll";
const char* TMP_GAME_DLL_PATH = "./data/code/yaeGame_temp.dll";

struct GameAPI
{
	u64 lastWriteTime = 0;
	void* libraryHandle = nullptr;
	GameFunctionPtr gameInit = nullptr;
	GameFunctionPtr gameUpdate = nullptr;
	GameFunctionPtr gameShutdown = nullptr;
	GameFunctionPtr onLibraryLoaded = nullptr;
	GameFunctionPtr onLibraryUnloaded = nullptr;
} s_gameAPI;

void loadGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	bool ret = platform::duplicateFile(GAME_DLL_PATH, TMP_GAME_DLL_PATH);
	YAE_ASSERT(ret);
	s_gameAPI.lastWriteTime = platform::getFileLastWriteTime(GAME_DLL_PATH);
	s_gameAPI.libraryHandle = platform::loadDynamicLibrary(TMP_GAME_DLL_PATH);
	YAE_ASSERT(s_gameAPI.libraryHandle);

	s_gameAPI.gameInit = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "initGame");
	s_gameAPI.gameUpdate = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "updateGame");
	s_gameAPI.gameShutdown = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "shutdownGame");
	s_gameAPI.onLibraryLoaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryLoaded");
	s_gameAPI.onLibraryUnloaded = (GameFunctionPtr)platform::getProcedureAddress(s_gameAPI.libraryHandle, "onLibraryUnloaded");

	YAE_ASSERT(s_gameAPI.gameInit);
	YAE_ASSERT(s_gameAPI.gameUpdate);
	YAE_ASSERT(s_gameAPI.gameShutdown);

	s_gameAPI.onLibraryLoaded();
}

void unloadGameAPI()
{
	YAE_CAPTURE_FUNCTION();
	
	s_gameAPI.onLibraryUnloaded();
	
	platform::unloadDynamicLibrary(s_gameAPI.libraryHandle);
	s_gameAPI = {};
}

void watchGameAPI()
{
	YAE_CAPTURE_FUNCTION();

	u64 lastWriteTime = platform::getFileLastWriteTime(GAME_DLL_PATH);
	if (lastWriteTime != s_gameAPI.lastWriteTime)
	{
		unloadGameAPI();
		loadGameAPI();
	}
}

void onGlfwKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS && mods & GLFW_MOD_CONTROL)
	{
		unloadGameAPI();
		loadGameAPI();
	}
}

void Application::init(const char* _name, u32 _width, u32 _height, char** _args, int _arg_count)
{
	YAE_CAPTURE_START("init");
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
	glfwSetKeyCallback(m_window, &onGlfwKeyEvent);

	YAE_ASSERT(m_window);
	YAE_VERBOSE_CAT("glfw", "Created glfw window");

	// Init Vulkan
	m_vulkanWrapper = context().defaultAllocator->create<VulkanRenderer>();

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	m_vulkanWrapper->init(m_window, enableValidationLayers);

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
	m_vulkanWrapper->initImGui();

	loadGameAPI();

	s_gameAPI.gameInit();

	YAE_CAPTURE_STOP("init");
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

			glfwPollEvents();
			ImGui_ImplGlfw_NewFrame();
			m_vulkanWrapper->beginFrame();
			ImGui::NewFrame();	
		}
		

		s_gameAPI.gameUpdate();

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

		m_vulkanWrapper->drawMesh();

		// Rendering
		ImGui::Render();
		ImDrawData* imguiDrawData = ImGui::GetDrawData();
		const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
		if (!isMinimized)
		{
			m_vulkanWrapper->drawImGui(imguiDrawData);
		}

		m_vulkanWrapper->endFrame();

		watchGameAPI();

		/*Time frameTime = clock.reset();
		printf("%f\n", 1.0 / frameTime.asSeconds64());*/

#if YAE_PROFILING_ENABLED
		context().profiler->update();
#endif

		YAE_CAPTURE_STOP("frame");
	}
	m_vulkanWrapper->waitIdle();
}

void Application::shutdown()
{
	s_gameAPI.gameShutdown();
	unloadGameAPI();

	m_vulkanWrapper->shutdownImGui();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imgui);
	m_imgui = nullptr;

	m_vulkanWrapper->shutdown();
	context().defaultAllocator->destroy<VulkanRenderer>(m_vulkanWrapper);
	m_vulkanWrapper = nullptr;

	glfwDestroyWindow(m_window);
	m_window = nullptr;
	YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");

	glfwTerminate();
	YAE_VERBOSE_CAT("glfw", "Terminated glfw");
}

} // namespace yae
